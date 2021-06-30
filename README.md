# 2D Feature Tracking Systems Comparison

<img src="images/keypoints.png" width="820" height="248" />

This project involved building feature tracking systems to test various detector, descriptor, and matcher combinations to see which ones perform best. This project consists of four parts:

* Loads KITTI image sequence, setting up data structures and putting everything into a ring buffer to optimize memory load. 
* Integrate several keypoint detectors such as SHI-TOMASI, HARRIS, FAST, BRISK, ORB, AKAZE, and SIFT and compare them with regard to number of keypoints and speed. 
* Implement descriptor extraction for BRISK, BRIEF, ORB, FREAK, AKAZE, SIFT and matching using brute force and also the FLANN approach. 
* Test the various algorithms in different combinations and compare them with regard to some performance measures. 

## Results

Various combinations of the 7 detectors and 6 descriptors chosen were run with a brute-force matcher using a k-nearest-neighbour selector and descriptor distance ratio of 0.8. The resulting number of features and time of computation for 10 image dataset is shown below. The top three combinations in terms of number of matches and computational time are BRIST-BRIEF, BRISK-BRISK, and BRISK-SIFT.

<img src="out/summaryResults.png" width="615" height="426" />

BRISK detector with BRIEF descriptor gives the most keypoint matches for the dataset, with 189 keypoint matches per frame on average in	24.1 ms across 10 images.

<img src="out/BRISK-BRIEF-MAT_BF-SEL_KNN-img1.jpg" width="1442" height="187" />

HARTRAK detector with BRISK descriptors gives the least keypoint matches for the dataset, with 16 keypoint matches per frame on average in	357.4 ms across 10 images.

<img src="out/HARRIS-BRISK-MAT_BF-SEL_KNN-img8.jpg" width="1442" height="187" />

## Dependencies for Running Locally
1. cmake >= 2.8
 * All OSes: [click here for installation instructions](https://cmake.org/install/)

2. make >= 4.1 (Linux, Mac), 3.81 (Windows)
 * Linux: make is installed by default on most Linux distros
 * Mac: [install Xcode command line tools to get make](https://developer.apple.com/xcode/features/)
 * Windows: [Click here for installation instructions](http://gnuwin32.sourceforge.net/packages/make.htm)

3. OpenCV >= 4.1
 * All OSes: refer to the [official instructions](https://docs.opencv.org/master/df/d65/tutorial_table_of_content_introduction.html)
 * This must be compiled from source using the `-D OPENCV_ENABLE_NONFREE=ON` cmake flag for testing the SIFT and SURF detectors. If using [homebrew](https://brew.sh/): `$> brew install --build-from-source opencv` will install required dependencies and compile opencv with the `opencv_contrib` module by default (no need to set `-DOPENCV_ENABLE_NONFREE=ON` manually). 
 * The OpenCV 4.1.0 source code can be found [here](https://github.com/opencv/opencv/tree/4.1.0)

4. gcc/g++ >= 5.4
  * Linux: gcc / g++ is installed by default on most Linux distros
  * Mac: same deal as make - [install Xcode command line tools](https://developer.apple.com/xcode/features/)
  * Windows: recommend using either [MinGW-w64](http://mingw-w64.org/doku.php/start) or [Microsoft's VCPKG, a C++ package manager](https://docs.microsoft.com/en-us/cpp/build/install-vcpkg?view=msvc-160&tabs=windows). VCPKG maintains its own binary distributions of OpenCV and many other packages. To see what packages are available, type `vcpkg search` at the command prompt. For example, once you've _VCPKG_ installed, you can install _OpenCV 4.1_ with the command:
```bash
c:\vcpkg> vcpkg install opencv4[nonfree,contrib]:x64-windows
```
Then, add *C:\vcpkg\installed\x64-windows\bin* and *C:\vcpkg\installed\x64-windows\debug\bin* to your user's _PATH_ variable. Also, set the _CMake Toolchain File_ to *c:\vcpkg\scripts\buildsystems\vcpkg.cmake*.

## Basic Build Instructions

1. Clone this repo.
2. Make a build directory in the top level directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./2D_feature_tracking`.
