#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>

using namespace cv;
using namespace std;

enum Direction {
    NORTH = 0,
    NORTH_EAST = 1,
    EAST = 2,
    SOUTH_EAST = 3,
    SOUTH = 4,
    SOUTH_WEST = 5,
    WEST = 6,
    NORTH_WEST = 7
};

struct ChainCode {
    Point start;
    unsigned int length = 0;
    vector<Direction> directions;
};

Point getNextPoint(const Point& current, Direction dir);
Direction getNextDirection(const Mat_<uchar>& image, Point current, Direction prevDir);
void drawContour();
ChainCode getChainCode(const Mat_<uchar>& image);
ChainCode getChainCodeDerivative(const ChainCode& chainCode);
void drawImageContour(const Mat_<uchar>& image, const ChainCode& chainCode, Point startPoint);
void testChainCode();
void reconstructFromFile();