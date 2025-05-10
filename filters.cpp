#include "stdafx.h"
#include "common.h"
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

Mat applyConvolution(const Mat& src, const Mat& kernel) {
    if (kernel.rows % 2 == 0 || kernel.cols % 2 == 0) {
        printf("Kernel dimensions must be odd.\n");
        return src.clone();
    }

    int kRows = kernel.rows;
    int kCols = kernel.cols;
    int kCenterX = kCols / 2;
    int kCenterY = kRows / 2;

    Mat dst = Mat::zeros(src.size(), CV_32F);

    float sumPositive = 0, sumNegative = 0;
    for (int i = 0; i < kRows; i++) {
        for (int j = 0; j < kCols; j++) {
            float val = kernel.at<float>(i, j);
            if (val > 0) sumPositive += val;
            else sumNegative -= val;
        }
    }

    float scaleFactor = 1.0f;
    bool isHighPass = false;

    float totalSum = sumPositive - sumNegative;
    if (abs(totalSum) < 1e-6) {
        isHighPass = true;
        scaleFactor = 1.0f / sumPositive;
    }
    else {
        scaleFactor = 1.0f / totalSum;
    }

    for (int i = kCenterY; i < src.rows - kCenterY; i++) {
        for (int j = kCenterX; j < src.cols - kCenterX; j++) {
            float sum = 0;

            for (int ki = -kCenterY; ki <= kCenterY; ki++) {
                for (int kj = -kCenterX; kj <= kCenterX; kj++) {
                    float kernelVal = kernel.at<float>(ki + kCenterY, kj + kCenterX);
                    float imageVal = src.at<uchar>(i + ki, j + kj);
                    sum += kernelVal * imageVal;
                }
            }

            if (isHighPass) {
                sum = sum * scaleFactor + 128;
            }
            else {
                sum *= scaleFactor;
            }

            dst.at<float>(i, j) = sum;
        }
    }

    Mat result;
    dst.convertTo(result, CV_8UC1);

    return result;
}

void applyPredefinedKernels(Mat& src) {

    // Arithmetic mean (3x3)
    Mat arithmeticMean3x3 = Mat::ones(3, 3, CV_32F) / 9.0f;

    // Gaussian (3x3)
    Mat gaussian3x3 = (Mat_<float>(3, 3) <<
        1, 2, 1,
        2, 4, 2,
        1, 2, 1) / 16.0f;

    // Laplacian 1 (3x3)
    Mat laplacian1 = (Mat_<float>(3, 3) <<
        0, -1, 0,
        -1, 4, -1,
        0, -1, 0);

    // Laplacian 2 (3x3) 
    Mat laplacian2 = (Mat_<float>(3, 3) <<
        -1, -1, -1,
        -1, 8, -1,
        -1, -1, -1);

    // High-pass 1 (3x3)
    Mat highPass1 = (Mat_<float>(3, 3) <<
        0, -1, 0,
        -1, 5, -1,
        0, -1, 0);

    // High-pass 2 (3x3)
    Mat highPass2 = (Mat_<float>(3, 3) <<
        -1, -1, -1,
        -1, 9, -1,
        -1, -1, -1);

    Mat result1 = applyConvolution(src, arithmeticMean3x3);
    Mat result2 = applyConvolution(src, gaussian3x3);
    Mat result3 = applyConvolution(src, laplacian1);
    Mat result4 = applyConvolution(src, laplacian2);
    Mat result5 = applyConvolution(src, highPass1);
    Mat result6 = applyConvolution(src, highPass2);

    imshow("Original", src);
    imshow("Arithmetic Mean (3x3)", result1);
    imshow("Gaussian (3x3)", result2);
    imshow("Laplacian 1", result3);
    imshow("Laplacian 2", result4);
    imshow("High-Pass 1", result5);
    imshow("High-Pass 2", result6);

    waitKey(0);
}

void testMeanFilter5x5(Mat& src) {
    Mat kernel = Mat::ones(5, 5, CV_32F) / 25.0f;
    Mat result = applyConvolution(src, kernel);

    imshow("Original", src);
    imshow("Mean Filter (5x5)", result);

    waitKey(0);
}

void testSpatialFiltering() {
    char fname[MAX_PATH];
    while (openFileDlg(fname)) {
        Mat src = imread(fname, IMREAD_GRAYSCALE);
        if (src.empty()) {
            printf("Could not open or find the image\n");
            continue;
        }

        applyPredefinedKernels(src);

        testMeanFilter5x5(src);

        break;
    }
}

void customKernelFiltering() {
    char fname[MAX_PATH];
    while (openFileDlg(fname)) {
        Mat src = imread(fname, IMREAD_GRAYSCALE);
        if (src.empty()) {
            printf("Could not open or find the image\n");
            continue;
        }

        int kernelSize;
        printf("Enter kernel size (odd number): ");
        scanf("%d", &kernelSize);

        if (kernelSize % 2 == 0) {
            printf("Kernel size must be odd. Using %d instead.\n", kernelSize + 1);
            kernelSize += 1;
        }

        Mat kernel = Mat::zeros(kernelSize, kernelSize, CV_32F);

        printf("Enter kernel values row by row:\n");
        for (int i = 0; i < kernelSize; i++) {
            for (int j = 0; j < kernelSize; j++) {
                float value;
                scanf("%f", &value);
                kernel.at<float>(i, j) = value;
            }
        }

        Mat result = applyConvolution(src, kernel);

        imshow("Original", src);
        imshow("Result", result);

        waitKey(0);
        break;
    }
}
