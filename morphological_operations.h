#pragma once
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat createStructuringElement(int type, int size);

Mat performDilation(const Mat& src, const Mat& element, int repetitions = 1);
Mat performErosion(const Mat& src, const Mat& element, int repetitions = 1);
Mat performOpening(const Mat& src, const Mat& element, int repetitions = 1);
Mat performClosing(const Mat& src, const Mat& element, int repetitions = 1);

Mat extractBoundary(const Mat& src, const Mat& element);
Mat fillRegion(const Mat& src, Point seedPoint);

void testDilation();
void testErosion();
void testOpening();
void testClosing();
void testBoundaryExtraction();
void testRegionFilling();