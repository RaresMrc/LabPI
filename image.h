#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <set>
#include <vector>

using namespace cv;
using namespace std;

// Structure to hold object properties
struct ObjectProps {
    int area;
    Point2f center;
    float orientation;
    float elongation;
    float thinnessFactor;
    vector<Point> contour;
    int perimeter;
};

ObjectProps computeObjectProperties(const Mat& labeledImg, int label);

void computeProjections(const Mat& labeledImg, int label, Mat& horizontalProjection, Mat& verticalProjection);

void selectObjectAndAnalyze();

void filterObjectsByAreaAndOrientation();