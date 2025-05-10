#include "stdafx.h"
#include "noise.h"
#include "common.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void medianFilter(const Mat& src, Mat& dst, int filterSize) {
    if (filterSize % 2 == 0) {
        printf("Filter size must be odd. Using %d instead.\n", filterSize + 1);
        filterSize += 1;
    }
    dst = src.clone();
    int halfSize = filterSize / 2;

    double t = (double)getTickCount();

    for (int i = halfSize; i < src.rows - halfSize; i++) {
        for (int j = halfSize; j < src.cols - halfSize; j++) {
            std::vector<uchar> neighborhood;

            for (int ki = -halfSize; ki <= halfSize; ki++) {
                for (int kj = -halfSize; kj <= halfSize; kj++) {
                    neighborhood.push_back(src.at<uchar>(i + ki, j + kj));
                }
            }

            std::sort(neighborhood.begin(), neighborhood.end());
            dst.at<uchar>(i, j) = neighborhood[neighborhood.size() / 2];
        }
    }

    t = ((double)getTickCount() - t) / getTickFrequency();
    printf("Median Filter %dx%d - Time = %.3f ms\n", filterSize, filterSize, t * 1000);
}

Mat createGaussianFilter(int size, double sigma) {
    Mat kernel(size, size, CV_32F);
    int center = size / 2;
    double sum = 0.0;

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int x = j - center;
            int y = i - center;
            kernel.at<float>(i, j) = (float)(exp(-(x * x + y * y) / (2 * sigma * sigma)) / (2 * M_PI * sigma * sigma));
            sum += kernel.at<float>(i, j);

        }
    }

    // Normalization
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            kernel.at<float>(i, j) /= sum;
        }
    }

    return kernel;
}

void gaussianFilter2D(Mat& src, Mat& dst, int filterSize) {
    if (filterSize % 2 == 0) {
        printf("Filter size must be odd, using %d instead \n", filterSize + 1);
        filterSize++;
    }

    double sigma = filterSize / 6.0f;
    Mat kernel = createGaussianFilter(filterSize, sigma);

    dst = Mat(src.size(), src.type());
    int halfSize = filterSize / 2;
    double t = (double)getTickCount();

    for (int i = halfSize; i < src.rows - halfSize; i++) {
        for (int j = halfSize; j < src.cols - halfSize; j++) {
            float sum = 0.0;

            for (int ki = -halfSize; ki <= halfSize; ki++) {
                for (int kj = -halfSize; kj <= halfSize; kj++) {
                    float kernelValue = kernel.at<float>(ki + halfSize, kj + halfSize);
                    uchar pixelValue = src.at<uchar>(i + ki, j + kj);
                    sum += kernelValue * pixelValue;
                }
            }

            dst.at<uchar>(i, j) = saturate_cast<uchar>(sum);
        }
    }

    t = ((double)getTickCount() - t) / getTickFrequency();
    printf("2D Gaussian Filter %dx%d (sigma=%.2f) - Time = %.3f ms\n", filterSize, filterSize, sigma, t * 1000);
}

void createGaussianKernel1D(int size, double sigma, Mat& kernelX, Mat& kernelY) {
    kernelX = Mat(1, size, CV_32F);
    kernelY = Mat(size, 1, CV_32F);
    int center = size / 2;
    double sumX = 0.0, sumY = 0.0;

    for (int i = 0; i < size; i++) {
        int x = i - center;
        float valueX = (float)(exp(-(x * x) / (2 * sigma * sigma)) / sqrt(2 * M_PI * sigma * sigma));
        float valueY = valueX;

        kernelX.at<float>(0, i) = valueX;
        kernelY.at<float>(i, 0) = valueY;

        sumX += valueX;
        sumY += valueY;
    }

    // Normalization
    for (int i = 0; i < size; i++) {
        kernelX.at<float>(0, i) /= (float)sumX;
        kernelY.at<float>(i, 0) /= (float)sumY;
    }
}

void separableGaussianFilter(const Mat& src, Mat& dst, int filterSize) {
    if (filterSize % 2 == 0) {
        printf("Filter size must be odd. Using %d instead.\n", filterSize + 1);
        filterSize += 1;
    }

    double sigma = filterSize / 6.0;
    Mat kernelX, kernelY;
    createGaussianKernel1D(filterSize, sigma, kernelX, kernelY);

    Mat temp = Mat::zeros(src.size(), CV_32F);
    dst = Mat::zeros(src.size(), src.type());
    int halfSize = filterSize / 2;

    double t = (double)getTickCount();

    for (int i = 0; i < src.rows; i++) {
        for (int j = halfSize; j < src.cols - halfSize; j++) {
            float sum = 0.0f;

            for (int k = -halfSize; k <= halfSize; k++) {
                sum += kernelX.at<float>(0, k + halfSize) * src.at<uchar>(i, j + k);
            }

            temp.at<float>(i, j) = sum;
        }
    }

    for (int i = halfSize; i < src.rows - halfSize; i++) {
        for (int j = halfSize; j < src.cols - halfSize; j++) {
            float sum = 0.0f;

            for (int k = -halfSize; k <= halfSize; k++) {
                sum += kernelY.at<float>(k + halfSize, 0) * temp.at<float>(i + k, j);
            }

            dst.at<uchar>(i, j) = saturate_cast<uchar>(sum);
        }
    }

    t = ((double)getTickCount() - t) / getTickFrequency();
    printf("Separable Gaussian Filter %dx%d (sigma=%.2f) - Time = %.3f ms\n",
        filterSize, filterSize, sigma, t * 1000);
}

void testNoiseFilters() {
    char fname[MAX_PATH];
    while (openFileDlg(fname)) {
        Mat src = imread(fname, IMREAD_GRAYSCALE);
        if (src.empty()) {
            printf("Could not open or find the image\n");
            continue;
        }

        int filterSize;
        printf("Enter filter size (3, 5, or 7): ");
        scanf("%d", &filterSize);

        if (filterSize != 3 && filterSize != 5 && filterSize != 7) {
            printf("Invalid filter size. Using 3 instead.\n");
            filterSize = 3;
        }

        // Display windows
        namedWindow("Original Image", WINDOW_AUTOSIZE);
        namedWindow("Median Filter", WINDOW_AUTOSIZE);
        namedWindow("2D Gaussian Filter", WINDOW_AUTOSIZE);
        namedWindow("Separable Gaussian Filter", WINDOW_AUTOSIZE);

        imshow("Original Image", src);

        // Median filter
        Mat medianResult;
        medianFilter(src, medianResult, filterSize);
        imshow("Median Filter", medianResult);

        // 2D Gaussian filter
        Mat gaussianResult;
        gaussianFilter2D(src, gaussianResult, filterSize);
        imshow("2D Gaussian Filter", gaussianResult);

        // Separable Gaussian filter
        Mat separableResult;
        separableGaussianFilter(src, separableResult, filterSize);
        imshow("Separable Gaussian Filter", separableResult);

        printf("\nProcessing times for filter size %dx%d:\n", filterSize, filterSize);

        waitKey(0);
        destroyAllWindows();
    }
}














