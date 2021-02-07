# 2D Feature Tracking Systems Comparison

<img src="images/keypoints.png" width="820" height="248" />

This project involved building feature tracking systems to test various detector, descriptor, and matcher combinations to see which ones perform best. This project consists of four parts:

* Loads KITTI image sequence, setting up data structures and putting everything into a ring buffer to optimize memory load. 
* Integrate several keypoint detectors such as SHI-TOMASI, HARRIS, FAST, BRISK, ORB, AKAZE, and SIFT and compare them with regard to number of keypoints and speed. 
* Implement descriptor extraction for BRISK, BRIEF, ORB, FREAK, AKAZE, SIFT and matching using brute force and also the FLANN approach. 
* Test the various algorithms in different combinations and compare them with regard to some performance measures. 

## Results

<img src="out/summaryResults.png" width="615" height="426" />
<img src="out/BRISK-BRIEF-MAT_BF-SEL_KNN-img1.jpg" width="1442" height="187" />
<img src="out/HARRIS-BRISK-MAT_BF-SEL_KNN-img8.jpg" width="1442" height="187" />



## Dependencies for Running Locally
* cmake >= 2.8
  * All OSes: [click here for installation instructions](https://cmake.org/install/)
* make >= 4.1 (Linux, Mac), 3.81 (Windows)
  * Linux: make is installed by default on most Linux distros
  * Mac: [install Xcode command line tools to get make](https://developer.apple.com/xcode/features/)
  * Windows: [Click here for installation instructions](http://gnuwin32.sourceforge.net/packages/make.htm)
* OpenCV >= 4.1
  * This must be compiled from source using the `-D OPENCV_ENABLE_NONFREE=ON` cmake flag for testing the SIFT and SURF detectors.
  * The OpenCV 4.1.0 source code can be found [here](https://github.com/opencv/opencv/tree/4.1.0)
* gcc/g++ >= 5.4
  * Linux: gcc / g++ is installed by default on most Linux distros
  * Mac: same deal as make - [install Xcode command line tools](https://developer.apple.com/xcode/features/)
  * Windows: recommend using [MinGW](http://www.mingw.org/)

## Basic Build Instructions

1. Clone this repo.
2. Make a build directory in the top level directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./2D_feature_tracking`.
