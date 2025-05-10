#include "stdafx.h"
#include "border_detection.h"
#include "common.h"
#include <fstream>

Point getNextPoint(const Point& current, Direction dir) {
    const int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
    const int dy[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };

    return Point(current.x + dx[dir], current.y + dy[dir]);
}

Direction getNextDirection(const Mat_<uchar>& image, Point current, Direction prevDir) {
    const int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
    const int dy[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };

    int startDir;
    if (prevDir % 2 == 0) {
        startDir = (prevDir + 7) % 8;
    }
    else {
        startDir = (prevDir + 6) % 8;
    }

    for (int i = 0; i < 8; i++) {
        Direction newDir = static_cast<Direction>((startDir + i) % 8);
        int newX = current.x + dx[newDir];
        int newY = current.y + dy[newDir];

        if (newX >= 0 && newX < image.cols && newY >= 0 && newY < image.rows) {
            if (image.at<uchar>(newY, newX) == 0) {
                return newDir;
            }
        }
    }

    return prevDir;
}

void drawContour() {
    char fname[MAX_PATH];
    if (!openFileDlg(fname)) {
        return;
    }

    Mat src = imread(fname, IMREAD_GRAYSCALE);
    if (src.empty()) {
        printf("Could not open or find the image\n");
        return;
    }

    Mat_<uchar> binaryImg = src.clone();

    Point startPoint(-1, -1);
    for (int y = 0; y < binaryImg.rows; y++) {
        for (int x = 0; x < binaryImg.cols; x++) {
            if (binaryImg.at<uchar>(y, x) == 0) {
                startPoint = Point(x, y);
                break;
            }
        }
        if (startPoint.x != -1) break;
    }

    if (startPoint.x == -1) {
        printf("No object found in the image\n");
        return;
    }

    Mat_<uchar> contourImg = Mat_<uchar>::zeros(binaryImg.size());

    Point P0 = startPoint;
    Direction dir = EAST;

    vector<Point> contourPoints;
    contourPoints.push_back(P0);

    Point currentPoint = P0;
    Point P1;
    bool firstStep = true;

    do {
        contourImg.at<uchar>(currentPoint.y, currentPoint.x) = 255;

        dir = getNextDirection(binaryImg, currentPoint, dir);

        currentPoint = getNextPoint(currentPoint, dir);

        if (firstStep) {
            P1 = currentPoint;
            firstStep = false;
        }

        contourPoints.push_back(currentPoint);

        if (contourPoints.size() > 2 && currentPoint == P1 && contourPoints[contourPoints.size() - 2] == P0) {
            break;
        }

    } while (currentPoint.x >= 0 && currentPoint.x < binaryImg.cols &&
        currentPoint.y >= 0 && currentPoint.y < binaryImg.rows);

    imshow("Original Image", binaryImg);
    imshow("Contour Image", contourImg);

    waitKey(0);
    destroyAllWindows();
}

ChainCode getChainedCode() {
    ChainCode chainCode;

    char fname[MAX_PATH];
    if (!openFileDlg(fname)) {
        return chainCode;
    }

    Mat src = imread(fname, IMREAD_GRAYSCALE);
    if (src.empty()) {
        printf("Could not open or find the image\n");
        return chainCode;
    }

    Mat_<uchar> binaryImg = src.clone();

    Point startPoint(-1, -1);
    for (int y = 0; y < binaryImg.rows; y++) {
        for (int x = 0; x < binaryImg.cols; x++) {
            if (binaryImg.at<uchar>(y, x) == 0) {
                startPoint = Point(x, y);
                break;
            }
        }
        if (startPoint.x != -1) break;
    }

    if (startPoint.x == -1) {
        printf("No object found in the image\n");
        return chainCode;
    }

    chainCode.start = startPoint;
    chainCode.directions.clear();

    Point P0 = startPoint;
    Direction dir = EAST;

    vector<Point> contourPoints;
    contourPoints.push_back(P0);

    Point currentPoint = P0;
    Point P1;
    bool firstStep = true;

    do {
        dir = getNextDirection(binaryImg, currentPoint, dir);

        chainCode.directions.push_back(dir);

        currentPoint = getNextPoint(currentPoint, dir);

        if (firstStep) {
            P1 = currentPoint;
            firstStep = false;
        }

        contourPoints.push_back(currentPoint);

        if (contourPoints.size() > 2 && currentPoint == P1 && contourPoints[contourPoints.size() - 2] == P0) {
            break;
        }

    } while (currentPoint.x >= 0 && currentPoint.x < binaryImg.cols &&
        currentPoint.y >= 0 && currentPoint.y < binaryImg.rows);

    chainCode.length = chainCode.directions.size();

    return chainCode;
}

ChainCode getChainedCodeDerivative(const ChainCode& chainCode) {
    ChainCode derivative;
    derivative.start = chainCode.start;
    derivative.directions.clear();

    if (chainCode.length <= 1) {
        derivative.length = 0;
        return derivative;
    }

    for (size_t i = 0; i < chainCode.length - 1; i++) {
        int diff = (static_cast<int>(chainCode.directions[i + 1]) - static_cast<int>(chainCode.directions[i]) + 8) % 8;
        derivative.directions.push_back(static_cast<Direction>(diff));
    }

    int diff = (static_cast<int>(chainCode.directions[0]) - static_cast<int>(chainCode.directions[chainCode.length - 1]) + 8) % 8;
    derivative.directions.push_back(static_cast<Direction>(diff));

    derivative.length = derivative.directions.size();

    return derivative;
}

void testChainCode() {
    ChainCode chainCode = getChainedCode();
    ChainCode derivative = getChainedCodeDerivative(chainCode);

    printf("\n---------------------------------- CHAIN CODE ---------------------------------------\n");
    printf("Starting point: (%d, %d)\n", chainCode.start.x, chainCode.start.y);
    printf("Chain code length: %u\n\n", chainCode.length);

    printf("Original Chain Code:\n");
    for (size_t i = 0; i < chainCode.length; i++) {
        printf("%d", static_cast<int>(chainCode.directions[i]));
        if ((i + 1) % 50 == 0) printf("\n");
        else if ((i + 1) % 10 == 0) printf(" ");
    }
    printf("\n\n");

    printf("Derivative Chain Code:\n");
    for (size_t i = 0; i < derivative.length; i++) {
        printf("%d", static_cast<int>(derivative.directions[i]));
        if ((i + 1) % 50 == 0) printf("\n");
        else if ((i + 1) % 10 == 0) printf(" ");
    }
    printf("\n\n");

    system("pause");
}

void drawImageContour(const Mat_<uchar>& image, const ChainCode& chainCode, Point startPoint) {
    Mat_<uchar> result = image.clone();

    Point currentPoint = startPoint;

    if (currentPoint.y >= 0 && currentPoint.y < result.rows &&
        currentPoint.x >= 0 && currentPoint.x < result.cols) {
        result.at<uchar>(currentPoint.y, currentPoint.x) = 255;
    }

    for (size_t i = 0; i < chainCode.length; i++) {
        currentPoint = getNextPoint(currentPoint, chainCode.directions[i]);

        if (currentPoint.y >= 0 && currentPoint.y < result.rows &&
            currentPoint.x >= 0 && currentPoint.x < result.cols) {
            result.at<uchar>(currentPoint.y, currentPoint.x) = 255;
        }
    }

    imshow("Original Image", image);
    imshow("Reconstructed Contour", result);
    waitKey(0);
    destroyAllWindows();
}

void reconstructFromFile() {
    Mat_<uchar> background = imread("C:\\Users\\rcm14\\Pictures\\test_images\\lab6\\gray_background.bmp", IMREAD_GRAYSCALE);
    if (background.empty()) {
        printf("Could not open gray_background.bmp\n");
        return;
    }

    ifstream file("C:\\Users\\rcm14\\Pictures\\test_images\\lab6\\reconstruct.txt");
    if (!file.is_open()) {
        printf("Could not open reconstruct.txt\n");
        return;
    }

    int row, col;
    file >> row >> col;
    Point startPoint(col, row);

    int numCodes;
    file >> numCodes;

    ChainCode chainCode;
    chainCode.start = startPoint;
    chainCode.directions.clear();

    int dir;
    while (file >> dir) {
        if (dir >= 0 && dir <= 7) {
            chainCode.directions.push_back(static_cast<Direction>(dir));
        }
    }

    file.close();

    chainCode.length = chainCode.directions.size();
    printf("Read %u directions from file\n", chainCode.length);

    drawImageContour(background, chainCode, startPoint);
}