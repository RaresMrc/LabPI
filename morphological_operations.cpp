#include "stdafx.h"
#include "morphological_operations.h"
#include "common.h"

Mat createStructuringElement(int type, int size) {
    Mat element = Mat::zeros(size, size, CV_8UC1);
    int center = size / 2;

    if (type == 4) {
        element.at<uchar>(center, center) = 255;
        if (center > 0) element.at<uchar>(center - 1, center) = 255;
        if (center < size - 1) element.at<uchar>(center + 1, center) = 255;
        if (center > 0) element.at<uchar>(center, center - 1) = 255;
        if (center < size - 1) element.at<uchar>(center, center + 1) = 255;
    }
    else if (type == 8) {
        for (int i = max(0, center - 1); i <= min(size - 1, center + 1); i++) {
            for (int j = max(0, center - 1); j <= min(size - 1, center + 1); j++) {
                element.at<uchar>(i, j) = 255;
            }
        }
    }

    return element;
}

Mat dilate(const Mat& src, const Mat& element) {
    Mat dst = Mat::ones(src.size(), CV_8UC1) * 255;

    int elementCenterY = element.rows / 2;
    int elementCenterX = element.cols / 2;

    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            if (src.at<uchar>(i, j) == 0) {
                for (int m = 0; m < element.rows; m++) {
                    for (int n = 0; n < element.cols; n++) {
                        if (element.at<uchar>(m, n) > 0) {
                            int ni = i + (m - elementCenterY);
                            int nj = j + (n - elementCenterX);

                            if (ni >= 0 && ni < dst.rows && nj >= 0 && nj < dst.cols) {
                                dst.at<uchar>(ni, nj) = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    return dst;
}

Mat erode(const Mat& src, const Mat& element) {
    Mat dst = Mat::ones(src.size(), CV_8UC1) * 255;

    int elementCenterY = element.rows / 2;
    int elementCenterX = element.cols / 2;

    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            bool keepAsObject = true;

            for (int m = 0; m < element.rows && keepAsObject; m++) {
                for (int n = 0; n < element.cols && keepAsObject; n++) {
                    if (element.at<uchar>(m, n) > 0) {
                        int si = i + (m - elementCenterY);
                        int sj = j + (n - elementCenterX);

                        if (si < 0 || si >= src.rows || sj < 0 || sj >= src.cols ||
                            src.at<uchar>(si, sj) != 0) {
                            keepAsObject = false;
                        }
                    }
                }
            }

            if (keepAsObject) {
                dst.at<uchar>(i, j) = 0;
            }
        }
    }

    return dst;
}

Mat opening(const Mat& src, const Mat& element) {
    Mat eroded = erode(src, element);
    Mat opened = dilate(eroded, element);
    return opened;
}

Mat closing(const Mat& src, const Mat& element) {
    Mat dilated = dilate(src, element);
    Mat closed = erode(dilated, element);
    return closed;
}

Mat performDilation(const Mat& src, const Mat& element, int repetitions) {
    Mat result = src.clone();
    for (int i = 0; i < repetitions; i++) {
        result = dilate(result, element);
    }
    return result;
}

Mat performErosion(const Mat& src, const Mat& element, int repetitions) {
    Mat result = src.clone();
    for (int i = 0; i < repetitions; i++) {
        result = erode(result, element);
    }
    return result;
}

Mat performOpening(const Mat& src, const Mat& element, int repetitions) {
    Mat result = src.clone();
    for (int i = 0; i < repetitions; i++) {
        result = opening(result, element);

        if (i > 0) {
            Mat prev = opening(src, element);
            for (int j = 0; j < i; j++) {
                prev = opening(prev, element);
            }

            Mat diff;
            compare(result, prev, diff, CMP_NE);
            if (countNonZero(diff) == 0) {
                printf("Opening reached idempotence after %d repetitions.\n", i + 1);
                break;
            }
        }
    }
    return result;
}

Mat performClosing(const Mat& src, const Mat& element, int repetitions) {
    Mat result = src.clone();
    for (int i = 0; i < repetitions; i++) {
        result = closing(result, element);

        if (i > 0) {
            Mat prev = closing(src, element);
            for (int j = 0; j < i; j++) {
                prev = closing(prev, element);
            }

            Mat diff;
            compare(result, prev, diff, CMP_NE);
            if (countNonZero(diff) == 0) {
                printf("Closing reached idempotence after %d repetitions.\n", i + 1);
                break;
            }
        }
    }
    return result;
}

Mat extractBoundary(const Mat& src, const Mat& element) {
    Mat eroded = erode(src, element);

    Mat boundary = Mat::ones(src.size(), CV_8UC1) * 255;
    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            if (src.at<uchar>(i, j) == 0 && eroded.at<uchar>(i, j) == 255) {
                boundary.at<uchar>(i, j) = 0;
            }
        }
    }

    return boundary;
}

Mat gSrc;
Point gSeedPoint(-1, -1);

void fillRegionCallback(int event, int x, int y, int flags, void* userdata) {
    if (event == EVENT_LBUTTONDOWN) {
        gSeedPoint = Point(x, y);

        Mat filled = fillRegion(gSrc, gSeedPoint);
        imshow("Filled Region", filled);
    }
}

Mat fillRegion(const Mat& src, Point seedPoint) {
    Mat X = Mat::ones(src.size(), CV_8UC1) * 255;
    X.at<uchar>(seedPoint.y, seedPoint.x) = 0;

    Mat element = createStructuringElement(8, 3);

    Mat srcComplement = Mat::zeros(src.size(), CV_8UC1);
    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            srcComplement.at<uchar>(i, j) = (src.at<uchar>(i, j) == 0) ? 255 : 0;
        }
    }

    Mat Xprev, Xcurrent = X.clone();
    int iterations = 0;
    bool converged = false;

    while (!converged && iterations < 100) {
        iterations++;
        Xprev = Xcurrent.clone();

        Mat dilated = dilate(Xprev, element);

        Xcurrent = Mat::ones(src.size(), CV_8UC1) * 255;
        for (int i = 0; i < src.rows; i++) {
            for (int j = 0; j < src.cols; j++) {
                // Intersection
                if (dilated.at<uchar>(i, j) == 0 && srcComplement.at<uchar>(i, j) == 0) {
                    Xcurrent.at<uchar>(i, j) = 0;
                }
            }
        }

        Mat diff;
        compare(Xcurrent, Xprev, diff, CMP_NE);
        if (countNonZero(diff) == 0) {
            converged = true;
            printf("Region filling converged after %d iterations.\n", iterations);
        }
    }

    Mat filled = Mat::ones(src.size(), CV_8UC1) * 255;
    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            // Union
            if (Xcurrent.at<uchar>(i, j) == 0 || src.at<uchar>(i, j) == 0) {
                filled.at<uchar>(i, j) = 0;
            }
        }
    }

    return filled;
}

void testDilation() {
    char fname[MAX_PATH];
    while (openFileDlg(fname)) {
        Mat src = imread(fname, IMREAD_GRAYSCALE);
        if (src.empty()) {
            printf("Could not open or find the image\n");
            continue;
        }

        threshold(src, src, 128, 255, THRESH_BINARY_INV);

        int connectivity;
        printf("Enter connectivity type (4 or 8): ");
        scanf("%d", &connectivity);

        int size;
        printf("Enter structuring element size (odd number): ");
        scanf("%d", &size);

        if (size % 2 == 0) {
            printf("Size must be odd. Using %d instead.\n", size + 1);
            size += 1;
        }

        Mat element = createStructuringElement(connectivity, size);

        int repetitions;
        printf("Enter number of repetitions: ");
        scanf("%d", &repetitions);

        imshow("Original Image", src);

        Mat dilated = performDilation(src, element, repetitions);
        imshow("Dilated Image", dilated);

        waitKey(0);
        destroyAllWindows();
    }
}

void testErosion() {
    char fname[MAX_PATH];
    while (openFileDlg(fname)) {
        Mat src = imread(fname, IMREAD_GRAYSCALE);
        if (src.empty()) {
            printf("Could not open or find the image\n");
            continue;
        }

        threshold(src, src, 128, 255, THRESH_BINARY_INV);

        int connectivity;
        printf("Enter connectivity type (4 or 8): ");
        scanf("%d", &connectivity);

        int size;
        printf("Enter structuring element size (odd number): ");
        scanf("%d", &size);

        if (size % 2 == 0) {
            printf("Size must be odd. Using %d instead.\n", size + 1);
            size += 1;
        }

        Mat element = createStructuringElement(connectivity, size);

        int repetitions;
        printf("Enter number of repetitions: ");
        scanf("%d", &repetitions);

        imshow("Original Image", src);

        Mat eroded = performErosion(src, element, repetitions);
        imshow("Eroded Image", eroded);

        waitKey(0);
        destroyAllWindows();
    }
}

void testOpening() {
    char fname[MAX_PATH];
    while (openFileDlg(fname)) {
        Mat src = imread(fname, IMREAD_GRAYSCALE);
        if (src.empty()) {
            printf("Could not open or find the image\n");
            continue;
        }

        threshold(src, src, 128, 255, THRESH_BINARY_INV);

        int connectivity;
        printf("Enter connectivity type (4 or 8): ");
        scanf("%d", &connectivity);

        int size;
        printf("Enter structuring element size (odd number): ");
        scanf("%d", &size);

        if (size % 2 == 0) {
            printf("Size must be odd. Using %d instead.\n", size + 1);
            size += 1;
        }

        Mat element = createStructuringElement(connectivity, size);

        int repetitions;
        printf("Enter number of repetitions: ");
        scanf("%d", &repetitions);

        imshow("Original Image", src);

        Mat opened = performOpening(src, element, repetitions);
        imshow("Opened Image", opened);

        waitKey(0);
        destroyAllWindows();
    }
}

void testClosing() {
    char fname[MAX_PATH];
    while (openFileDlg(fname)) {
        Mat src = imread(fname, IMREAD_GRAYSCALE);
        if (src.empty()) {
            printf("Could not open or find the image\n");
            continue;
        }

        threshold(src, src, 128, 255, THRESH_BINARY_INV);

        int connectivity;
        printf("Enter connectivity type (4 or 8): ");
        scanf("%d", &connectivity);

        int size;
        printf("Enter structuring element size (odd number): ");
        scanf("%d", &size);

        if (size % 2 == 0) {
            printf("Size must be odd. Using %d instead.\n", size + 1);
            size += 1;
        }

        Mat element = createStructuringElement(connectivity, size);

        int repetitions;
        printf("Enter number of repetitions: ");
        scanf("%d", &repetitions);

        imshow("Original Image", src);

        Mat closed = performClosing(src, element, repetitions);
        imshow("Closed Image", closed);

        waitKey(0);
        destroyAllWindows();
    }
}

void testBoundaryExtraction() {
    char fname[MAX_PATH];
    while (openFileDlg(fname)) {
        Mat src = imread(fname, IMREAD_GRAYSCALE);
        if (src.empty()) {
            printf("Could not open or find the image\n");
            continue;
        }

        threshold(src, src, 128, 255, THRESH_BINARY_INV);

        int connectivity;
        printf("Enter connectivity type (4 or 8): ");
        scanf("%d", &connectivity);

        int size;
        printf("Enter structuring element size (odd number): ");
        scanf("%d", &size);

        if (size % 2 == 0) {
            printf("Size must be odd. Using %d instead.\n", size + 1);
            size += 1;
        }

        Mat element = createStructuringElement(connectivity, size);

        imshow("Original Image", src);

        Mat boundary = extractBoundary(src, element);
        imshow("Boundary Image", boundary);

        waitKey(0);
        destroyAllWindows();
    }
}

void testRegionFilling() {
    char fname[MAX_PATH];
    while (openFileDlg(fname)) {
        Mat src = imread(fname, IMREAD_GRAYSCALE);
        if (src.empty()) {
            printf("Could not open or find the image\n");
            continue;
        }

        threshold(src, src, 128, 255, THRESH_BINARY_INV);

        imshow("Original Image", src);

        gSrc = src.clone();
        gSeedPoint = Point(-1, -1);

        printf("Click inside a region to fill it.\n");
        imshow("Click to Fill Region", gSrc);
        setMouseCallback("Click to Fill Region", fillRegionCallback);

        waitKey(0);
        destroyAllWindows();
    }
}