#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <set>
#include <vector>

using namespace cv;
using namespace std;

Mat_<uchar> labelImageLateralTraversal();

Mat_<Vec3b> labelImageColoring(const Mat_<uchar>& labeledImg);

Mat_<uchar> labelImageTwoPass();