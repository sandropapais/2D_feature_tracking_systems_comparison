#include <numeric>
#include "matching2D.hpp"

using namespace std;

            //// TASK MP.5 -> add FLANN matching in file matching2D.cpp
            //// TASK MP.6 -> add KNN match selection and perform descriptor distance ratio filtering with t=0.8 in file matching2D.cpp

// Find best matches for keypoints in two camera images based on several matching methods
void matchDescriptors(std::vector<cv::KeyPoint> &kPtsSource, std::vector<cv::KeyPoint> &kPtsRef, cv::Mat &descSource, cv::Mat &descRef,
                      std::vector<cv::DMatch> &matches, std::string descriptorType, std::string matcherType, std::string selectorType)
{
    // configure matcher
    bool crossCheck = false;
    cv::Ptr<cv::DescriptorMatcher> matcher;

    if (matcherType.compare("MAT_BF") == 0)
    {
        int normType = descriptorType.compare("DES_BINARY") == 0 ? cv::NORM_HAMMING : cv::NORM_L2;
        matcher = cv::BFMatcher::create(normType, crossCheck);
        cout << "BF matching, cross-check=" << crossCheck << ", ";
    }
    else if (matcherType.compare("MAT_FLANN") == 0)
    {
        if (descSource.type() != CV_32F)
        { // OpenCV bug workaround : convert binary descriptors to floating point due to a bug in current OpenCV implementation
            descSource.convertTo(descSource, CV_32F);
            descRef.convertTo(descRef, CV_32F);
        }
        matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
        cout << "FLANN matching, ";
    }

    // perform matching task
    if (selectorType.compare("SEL_NN") == 0)
    { // nearest neighbor (best match)
        double t = (double)cv::getTickCount();
        matcher->match(descSource, descRef, matches); // Finds the best match for each descriptor in desc1
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        cout << "NN selection, with n=" << matches.size() << "descriptor matches in " << 1000 * t / 1.0 << " ms" << endl;
        writeMatchingResults(matches.size(), 1000 * t / 1.0 );
    }
    else if (selectorType.compare("SEL_KNN") == 0)
    { // k nearest neighbors (k=2)
        vector<vector<cv::DMatch>> knn_matches;
        double t = (double)cv::getTickCount();
        matcher->knnMatch(descSource, descRef, knn_matches, 2); // finds the 2 best matches

        // filter matches using descriptor distance ratio test
        double minDescDistRatio = 0.8;
        for (auto it = knn_matches.begin(); it != knn_matches.end(); ++it)
        {
            if ((*it)[0].distance < minDescDistRatio * (*it)[1].distance)
            {
                matches.push_back((*it)[0]);
            }
        }
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        cout << "KNN selection, with n=" << matches.size() << " descriptor matches in " << 1000 * t / 1.0 << " ms" << endl;
        cout << "Keypoints removed by distance ratio check = " << knn_matches.size() - matches.size() << endl;
        writeMatchingResults(matches.size(), 1000 * t / 1.0 );
    }
}

// Use one of several types of state-of-art descriptors to uniquely identify keypoints
void descKeypoints(vector<cv::KeyPoint> &keypoints, cv::Mat &img, cv::Mat &descriptors, string descriptorType)
{
    // select appropriate descriptor
    cv::Ptr<cv::DescriptorExtractor> extractor;
    if (descriptorType.compare("BRISK") == 0)
    {
        int threshold = 30;        // FAST/AGAST detection threshold score.
        int octaves = 3;           // detection octaves (use 0 to do single scale)
        float patternScale = 1.0f; // apply this scale to the pattern used for sampling the neighbourhood of a keypoint.

        extractor = cv::BRISK::create(threshold, octaves, patternScale);
    }
    else if (descriptorType.compare("BRIEF") == 0)
    {
        int bytes = 32; // legth of the descriptor in bytes, valid values are: 16, 32, or 64 
		bool use_orientation = false; // sample patterns using keypoints orientation

        extractor = cv::xfeatures2d::BriefDescriptorExtractor::create(bytes, use_orientation);
    }
    else if (descriptorType.compare("ORB") == 0)
    {
        int nfeatures = 500; // max features to retain
        float scaleFactor = 1.2f; // pyramid decimation ratio, greater than 1
        int nlevels = 8; // number of pyramid levels
        int edgeThreshold = 31; // size of border where no features are detected
        int firstLevel = 0; // level of pyramid to put source image to
        int WTA_K = 2; // number of points that produce each element of the oriented BRIEF descriptor
        cv::ORB::ScoreType scoreType = cv::ORB::HARRIS_SCORE; // score used to rank features
        int patchSize = 31; // size of patch used by oriented BREF descriptor
        int threshold = 20; // fast detection threshold score

        extractor = cv::ORB::create(nfeatures, scaleFactor, nlevels, edgeThreshold, firstLevel, WTA_K, scoreType, patchSize, threshold);
    }
    else if (descriptorType.compare("FREAK") == 0)
    {
        bool orientationNormalized = true; // Enable orientation normalization
		bool scaleNormalized = true; // Enable scale normalization
		float patternScale = 22.0f; // Scaling of the description pattern
		int nOctaves = 4; // Number of octaves covered by the detected keypoints
		// const std::vector<int> & selectedPairs = std::vector<int>(); // (Optional) user defined selected pairs indexes

        extractor = cv::xfeatures2d::FREAK::create(orientationNormalized, scaleNormalized, patternScale, nOctaves);
    }
    else if (descriptorType.compare("AKAZE") == 0)
    {
        cv::AKAZE::DescriptorType descriptor_type = cv::AKAZE::DESCRIPTOR_MLDB; // type of extracted descriptor
		int  	descriptor_size = 0; // size of descriptor in bits (0=full size)
		int  	descriptor_channels = 3; // number of channels in descriptor (1, 2, 3)
		float  	threshold_f = 0.001f; // detector response threshold to accept point
		int  	nOctaves = 4; // max octave evolution of image
		int  	nOctaveLayers = 4; // default number of sublevels per scale level
		cv::KAZE::DiffusivityType diffusivity = cv::KAZE::DIFF_PM_G2; // diffusivity type

        extractor = cv::AKAZE::create(descriptor_type, descriptor_size, descriptor_channels, threshold_f, nOctaves, nOctaveLayers, diffusivity);
    }
    else if (descriptorType.compare("SIFT") == 0)
    {
        int nfeatures = 0; // number of best features to retain
        int nOctaveLayers = 3; // number of layers in each octave
        double contrastThreshold = 0.06; // used to filter out weak features from low contrast regions (larger thresh = less feats)
        double edgeThreshold = 10; // used to filter out edge-like features (larger thresh = more feats)
        double sigma = 1.6; // sigma of gaussian applied to image at octave #0

        extractor = cv::xfeatures2d::SIFT::create(nfeatures, nOctaveLayers, contrastThreshold, edgeThreshold, sigma);
    }

    // perform feature description
    double t = (double)cv::getTickCount();
    extractor->compute(img, keypoints, descriptors);
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << descriptorType << " descriptor extraction in " << 1000 * t / 1.0 << " ms" << endl;
    writeDescriptionResults(1000 * t / 1.0 );
}

// Detect keypoints in image using the traditional Shi-Thomasi detector
void detKeypointsShiTomasi(vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis)
{
    // compute detector parameters based on image size
    int blockSize = 4;       //  size of an average block for computing a derivative covariation matrix over each pixel neighborhood
    double maxOverlap = 0.0; // max. permissible overlap between two features in %
    double minDistance = (1.0 - maxOverlap) * blockSize;
    int maxCorners = img.rows * img.cols / max(1.0, minDistance); // max. num. of keypoints

    double qualityLevel = 0.01; // minimal accepted quality of image corners
    double k = 0.04;

    // Apply corner detection
    double t = (double)cv::getTickCount();
    vector<cv::Point2f> corners;
    cv::goodFeaturesToTrack(img, corners, maxCorners, qualityLevel, minDistance, cv::Mat(), blockSize, false, k);

    // add corners to result vector
    for (auto it = corners.begin(); it != corners.end(); ++it)
    {

        cv::KeyPoint newKeyPoint;
        newKeyPoint.pt = cv::Point2f((*it).x, (*it).y);
        newKeyPoint.size = blockSize;
        keypoints.push_back(newKeyPoint);
    }
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << "Shi-Tomasi detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;
    writeDetectionResults(keypoints.size(),1000 * t / 1.0);

    // visualize results
    if (bVis)
    {
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        string windowName = "Shi-Tomasi Corner Detector Results";
        cv::namedWindow(windowName, 6);
        imshow(windowName, visImage);
        cv::waitKey(0);
    }
}

// Detect keypoints in image using the traditional Shi-Thomasi detector
void detKeypointsHarris(vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis)
{
    // Detector parameters
    int blockSize = 2;     // for every pixel, a blockSize × blockSize neighborhood is considered
    int apertureSize = 3;  // aperture parameter for Sobel operator (must be odd)
    int minResponse = 100; // minimum value for a corner in the 8bit scaled response matrix
    double k = 0.04;       // Harris parameter (see equation for details)

    // Detect Harris corners and normalize output
    double t = (double)cv::getTickCount();
    cv::Mat dst, dst_norm, dst_norm_scaled;
    dst = cv::Mat::zeros(img.size(), CV_32FC1);
    cv::cornerHarris(img, dst, blockSize, apertureSize, k, cv::BORDER_DEFAULT);
    cv::normalize(dst, dst_norm, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
    cv::convertScaleAbs(dst_norm, dst_norm_scaled);

    // Look for prominent corners and instantiate keypoints
    double maxOverlap = 0; // max. permissible overlap between two features in %, used during non-maxima suppression
    for (size_t j = 0; j < dst_norm.rows; j++)
    {
        for (size_t i = 0; i < dst_norm.cols; i++)
        {
            int response = (int)dst_norm.at<float>(j, i);
            if (response > minResponse)
            { // only store points above a threshold

                cv::KeyPoint newKeyPoint;
                newKeyPoint.pt = cv::Point2f(i, j);
                newKeyPoint.size = 2 * apertureSize;
                newKeyPoint.response = response;

                // perform non-maximum suppression (NMS) in local neighbourhood around new key point
                bool bOverlap = false;
                for (auto it = keypoints.begin(); it != keypoints.end(); ++it)
                {
                    double kptOverlap = cv::KeyPoint::overlap(newKeyPoint, *it);
                    if (kptOverlap > maxOverlap)
                    {
                        bOverlap = true;
                        if (newKeyPoint.response > (*it).response)
                        {                      // if overlap is >t AND response is higher for new kpt
                            *it = newKeyPoint; // replace old key point with new one
                            break;             // quit loop over keypoints
                        }
                    }
                }
                if (!bOverlap)
                {                                     // only add new key point if no overlap has been found in previous NMS
                    keypoints.push_back(newKeyPoint); // store new keypoint in dynamic list
                }
            }
        } // eof loop over cols
    }     // eof loop over rows

    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << "Harris detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;
    writeDetectionResults(keypoints.size(),1000 * t / 1.0);

    // visualize results
    if (bVis)
    {
        // cv::Mat visImage = dst_norm_scaled.clone();
        // cv::drawKeypoints(dst_norm_scaled, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        string windowName = "Harris Corner Detector Results";
        cv::namedWindow(windowName, 6);
        imshow(windowName, visImage);
        cv::waitKey(0);
    }
}

void detKeypointsModern(vector<cv::KeyPoint> &keypoints, cv::Mat &img, string detectorType, bool bVis)
{
    // shared variables declaration
    double t;
    cv::Ptr<cv::FeatureDetector> detector;

    if (detectorType.compare("FAST") == 0)
    {
        int threshold = 30; // difference between intensity of the central pixel and pixels of a circle around this pixel
        bool bNMS = true; // perform non-maxima suppression on keypoints
        cv::FastFeatureDetector::DetectorType type = cv::FastFeatureDetector::TYPE_9_16; // TYPE_9_16, TYPE_7_12, TYPE_5_8
        
        detector = cv::FastFeatureDetector::create(threshold, bNMS, type);

        t = (double)cv::getTickCount();
        detector->detect(img, keypoints);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        cout << detectorType << " detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;
        writeDetectionResults(keypoints.size(),1000 * t / 1.0);
    }
    else if (detectorType.compare("BRISK") == 0)
    {
        int threshold = 30; // AGAST detection threshold score
        int octaves = 3; // detection octaves, use 0 to do single scale
        float patternScale = 1.0f; // apply this scale to the pattern used for samping the neighbourhood of a keypount

        detector = cv::BRISK::create(threshold, octaves, patternScale);

        t = (double)cv::getTickCount();
        detector->detect(img, keypoints);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        cout << detectorType << " detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;
        writeDetectionResults(keypoints.size(),1000 * t / 1.0);
    }
    else if (detectorType.compare("ORB") == 0)
    {
        int nfeatures = 500; // max features to retain
        float scaleFactor = 1.2f; // pyramid decimation ratio, greater than 1
        int nlevels = 8; // number of pyramid levels
        int edgeThreshold = 31; // size of border where no features are detected
        int firstLevel = 0; // level of pyramid to put source image to
        int WTA_K = 2; // number of points that produce each element of the oriented BRIEF descriptor
        cv::ORB::ScoreType scoreType = cv::ORB::HARRIS_SCORE; // score used to rank features
        int patchSize = 31; // size of patch used by oriented BREF descriptor
        int threshold = 20; // fast detection threshold score

        detector = cv::ORB::create(nfeatures, scaleFactor, nlevels, edgeThreshold, firstLevel, WTA_K, scoreType, patchSize, threshold);

        t = (double)cv::getTickCount();
        detector->detect(img, keypoints);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        cout << detectorType << " detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;
        writeDetectionResults(keypoints.size(),1000 * t / 1.0);
    }
    else if (detectorType.compare("AKAZE") == 0)
    {
        cv::AKAZE::DescriptorType descriptor_type = cv::AKAZE::DESCRIPTOR_MLDB; // type of extracted descriptor
		int  	descriptor_size = 0; // size of descriptor in bits (0=full size)
		int  	descriptor_channels = 3; // number of channels in descriptor (1, 2, 3)
		float  	threshold_f = 0.001f; // detector response threshold to accept point
		int  	nOctaves = 4; // max octave evolution of image
		int  	nOctaveLayers = 4; // default number of sublevels per scale level
		cv::KAZE::DiffusivityType diffusivity = cv::KAZE::DIFF_PM_G2; // diffusivity type

        detector = cv::AKAZE::create(descriptor_type, descriptor_size, descriptor_channels, threshold_f, nOctaves, nOctaveLayers, diffusivity);

        t = (double)cv::getTickCount();
        detector->detect(img, keypoints);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        cout << detectorType << " detection with n= " << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;
        writeDetectionResults(keypoints.size(),1000 * t / 1.0);
    }
    else if (detectorType.compare("SIFT") == 0)
    {
        int nfeatures = 0; // number of best features to retain
        int nOctaveLayers = 3; // number of layers in each octave
        double contrastThreshold = 0.06; // used to filter out weak features from low contrast regions (larger thresh = less feats)
        double edgeThreshold = 10; // used to filter out edge-like features (larger thresh = more feats)
        double sigma = 1.6; // sigma of gaussian applied to image at octave #0

        detector = cv::xfeatures2d::SIFT::create(nfeatures, nOctaveLayers, contrastThreshold, edgeThreshold, sigma);

        t = (double)cv::getTickCount();
        detector->detect(img, keypoints);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        cout << detectorType << " detection with n= " << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;
        writeDetectionResults(keypoints.size(), 1000 * t / 1.0);
    }

    // visualize results
    if (bVis)
    {
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        string windowName = "Harris Corner Detector Results";
        cv::namedWindow(windowName, 6);
        imshow(windowName, visImage);
        cv::waitKey(0);
    }
}

void writeDetectionResults(float keypointsSize, double t)
{
    // Save results to workspace
    ofstream outfile;
    string outfileName = "../out/detectionResults.csv";
    outfile.open(outfileName, ofstream::app);
    if (outfile.is_open())
    {
        outfile << "," << keypointsSize << "," << t;
        outfile.close();
    }
    else cout << "[Error]: Unable to open file" << endl;
}

void writeDescriptionResults(double t)
{
    // Save results to workspace
    ofstream outfile;
    string outfileName = "../out/descriptionResults.csv";
    outfile.open(outfileName, ofstream::app);
    if (outfile.is_open())
    {
        outfile << "," << t;
        outfile.close();
    }
    else cout << "[Error]: Unable to open file" << endl;
}

void writeMatchingResults(float matchesSize, double t)
{
    // Save results to workspace
    ofstream outfile;
    string outfileName = "../out/matchingResults.csv";
    outfile.open(outfileName, ofstream::app);
    if (outfile.is_open())
    {
        outfile << "," << matchesSize << "," << t;
        outfile.close();
    }
    else cout << "[Error]: Unable to open file" << endl;
}

void iniResults(string fileName, string rowName)
{
    // Save results to workspace
    ofstream outfile;
    outfile.open(fileName, ofstream::app);
    if (outfile.is_open())
    {
        outfile << rowName;
        outfile.close();
    }
    else cout << "[Error]: Unable to open file" << endl;
}

void endResults(string fileName)
{
    // Save results to workspace
    ofstream outfile;
    outfile.open(fileName, ofstream::app);
    if (outfile.is_open())
    {
        outfile << "\n";
        outfile.close();
    }
    else cout << "[Error]: Unable to open file" << endl;
}


