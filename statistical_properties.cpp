#include "stdafx.h"
#include "statistical_properties.h"
#include <cmath>

using namespace cv;


void computeImageStatistics()
{
    char fname[MAX_PATH];
    while (openFileDlg(fname))
    {
        Mat src = imread(fname, IMREAD_GRAYSCALE);
        if (src.empty()) {
            printf("Could not load image\n");
            continue;
        }

        int histogram[256] = { 0 };
        for (int i = 0; i < src.rows; i++) {
            for (int j = 0; j < src.cols; j++) {
                histogram[src.at<uchar>(i, j)]++;
            }
        }

        double mean = 0.0;
        int totalPixels = src.rows * src.cols;
        for (int g = 0; g < 256; g++) {
            mean += g * histogram[g];
        }
        mean /= totalPixels;

        double deviation = 0.0;
        for (int g = 0; g < 256; g++) {
            deviation += (g - mean) * (g - mean) * histogram[g];
        }
        deviation /= totalPixels;
        double standard_deviation = sqrt(deviation);

        int cumulative[256] = { 0 };
        cumulative[0] = histogram[0];
        for (int g = 1; g < 256; g++) {
            cumulative[g] = cumulative[g - 1] + histogram[g];
        }

        printf("Mean: %.2f\n", mean);
        printf("Standard Deviation: %.2f\n", standard_deviation);

        showHistogram("Histogram", histogram, 256, 256);
        showHistogram("Cumulative Histogram", cumulative, 256, 256);

        imshow("Original Image", src);
        waitKey();
    }
}

void automaticGlobalBinarization()
{
    char fname[MAX_PATH];
    while (openFileDlg(fname))
    {
        Mat src = imread(fname, IMREAD_GRAYSCALE);
        if (src.empty()) {
            printf("Could not load image\n");
            continue;
        }

        // Get histogram
        int histogram[256] = { 0 };
        for (int i = 0; i < src.rows; i++) {
            for (int j = 0; j < src.cols; j++) {
                histogram[src.at<uchar>(i, j)]++;
            }
        }

        // Find minimum and maximum intensity
        int Imin = 0, Imax = 255;
        for (int g = 0; g < 256; g++) {
            if (histogram[g] > 0) {
                Imin = g;
                break;
            }
        }
        for (int g = 255; g >= 0; g--) {
            if (histogram[g] > 0) {
                Imax = g;
                break;
            }
        }

        // Threshold
        int T = (Imin + Imax) / 2;
        int T_prev = -1;
        double error = 0.1;

        // Calculate threshold
        while (abs(T - T_prev) > error) {
            T_prev = T;

            double sum1 = 0, count1 = 0;
            double sum2 = 0, count2 = 0;

            for (int g = Imin; g <= T; g++) {
                sum1 += g * histogram[g];
                count1 += histogram[g];
            }

            for (int g = T + 1; g <= Imax; g++) {
                sum2 += g * histogram[g];
                count2 += histogram[g];
            }

            double mu1 = (count1 > 0) ? sum1 / count1 : 0;
            double mu2 = (count2 > 0) ? sum2 / count2 : 0;

            T = (int)((mu1 + mu2) / 2);
        }

        printf("Computed threshold T = %d\n", T);

        // Binarization, if pixel value > T then make it white, otherwise black
        Mat dst = src.clone();
        for (int i = 0; i < src.rows; i++) {
            for (int j = 0; j < src.cols; j++) {
                dst.at<uchar>(i, j) = (src.at<uchar>(i, j) > T) ? 255 : 0;
            }
        }

        imshow("Original Image", src);
        imshow("Binarized Image", dst);
        showHistogram("Histogram", histogram, 256, 256);
        waitKey();
    }
}

void histogramTransformations()
{
    char fname[MAX_PATH];
    while (openFileDlg(fname))
    {
        Mat src = imread(fname, IMREAD_GRAYSCALE);
        if (src.empty()) {
            printf("Could not load image\n");
            continue;
        }

        int choice;
        printf("\nHistogram Transformation Options:\n");
        printf("1 - Negative\n");
        printf("2 - Contrast stretching/shrinking\n");
        printf("3 - Gamma correction\n");
        printf("4 - Brightness modification\n");
        printf("Choose option: ");
        scanf("%d", &choice);

        Mat dst = Mat(src.rows, src.cols, CV_8UC1);
        int histogram_src[256] = { 0 };
        int histogram_dst[256] = { 0 };

        // Calculate histogram
        for (int i = 0; i < src.rows; i++) {
            for (int j = 0; j < src.cols; j++) {
                histogram_src[src.at<uchar>(i, j)]++;
            }
        }

        switch (choice) {
        // Negative
        case 1: {
            for (int i = 0; i < src.rows; i++) {
                for (int j = 0; j < src.cols; j++) {
                    dst.at<uchar>(i, j) = 255 - src.at<uchar>(i, j);
                }
            }
            printf("Applied negative transformation\n");
            break;
        }

        // Contrast Stretching/Shrinking
        case 2: {
            int goutMIN, goutMAX;
            printf("Enter GOut min (0-255): ");
            scanf("%d", &goutMIN);
            printf("Enter GOut max (%d-255): ", goutMIN);
            scanf("%d", &goutMAX);

            int ginMIN = 0, ginMAX = 255;
            for (int g = 0; g < 256; g++) {
                if (histogram_src[g] > 0) {
                    ginMIN = g;
                    break;
                }
            }
            for (int g = 255; g >= 0; g--) {
                if (histogram_src[g] > 0) {
                    ginMAX = g;
                    break;
                }
            }

            for (int i = 0; i < src.rows; i++) {
                for (int j = 0; j < src.cols; j++) {
                    int gin = src.at<uchar>(i, j);
                    int gout = goutMIN + (gin - ginMIN) * (goutMAX - goutMIN) / (ginMAX - ginMIN);
                    dst.at<uchar>(i, j) = max(0, min(255, gout));
                }
            }
            printf("Applied contrast transformation (GIn min=%d, GIn max=%d, GOut min=%d, GOut max=%d)\n",
                ginMIN, ginMAX, goutMIN, goutMAX);
            break;
        }
        
	    // Gamma correction
        case 3: {
            double gamma;
            printf("Enter gamma value: ");
            scanf("%lf", &gamma);

            for (int i = 0; i < src.rows; i++) {
                for (int j = 0; j < src.cols; j++) {
                    double normalized = src.at<uchar>(i, j) / 255.0;
                    double transformed = pow(normalized, gamma);
                    dst.at<uchar>(i, j) = (uchar)(transformed * 255);
                }
            }
            printf("Applied gamma correction with gamma=%.2f\n", gamma);
            break;
        }

        // Brightness modification
        case 4: {
            int offset;
            printf("Enter brightness offset (-255 to 255): ");
            scanf("%d", &offset);

            for (int i = 0; i < src.rows; i++) {
                for (int j = 0; j < src.cols; j++) {
                    int val = src.at<uchar>(i, j) + offset;
                    dst.at<uchar>(i, j) = max(0, min(255, val));
                }
            }
            printf("Applied brightness modification with offset=%d\n", offset);
            break;
        }

        default:
            printf("Invalid choice\n");
            continue;
        }

        for (int i = 0; i < dst.rows; i++) {
            for (int j = 0; j < dst.cols; j++) {
                histogram_dst[dst.at<uchar>(i, j)]++;
            }
        }

        imshow("Source image", src);
        imshow("Transformed image", dst);
        showHistogram("Source histogram", histogram_src, 256, 256);
        showHistogram("Destination histogram", histogram_dst, 256, 256);
        waitKey();
    }
}

// Equalization
void histogramEqualization()
{
    char fname[MAX_PATH];
    while (openFileDlg(fname))
    {
        Mat src = imread(fname, IMREAD_GRAYSCALE);
        if (src.empty()) {
            printf("Could not load image\n");
            continue;
        }

        // Calculate histogram
        int histogram[256] = { 0 };
        for (int i = 0; i < src.rows; i++) {
            for (int j = 0; j < src.cols; j++) {
                histogram[src.at<uchar>(i, j)]++;
            }
        }

        // Calculate probability function
        double pdf[256];
        int totalPixels = src.rows * src.cols;
        for (int g = 0; g < 256; g++) {
            pdf[g] = (double)histogram[g] / totalPixels;
        }

        // Calculate cumulative distribution
        double cdf[256];
        cdf[0] = pdf[0];
        for (int g = 1; g < 256; g++) {
            cdf[g] = cdf[g - 1] + pdf[g];
        }

        int tab[256];
        for (int g = 0; g < 256; g++) {
            tab[g] = (int)(255 * cdf[g]);
        }

        Mat dst = Mat(src.rows, src.cols, CV_8UC1);
        for (int i = 0; i < src.rows; i++) {
            for (int j = 0; j < src.cols; j++) {
                dst.at<uchar>(i, j) = tab[src.at<uchar>(i, j)];
            }
        }

        // Calculate equalized histogram
        int histogram_eq[256] = { 0 };
        for (int i = 0; i < dst.rows; i++) {
            for (int j = 0; j < dst.cols; j++) {
                histogram_eq[dst.at<uchar>(i, j)]++;
            }
        }

        imshow("Original Image", src);
        imshow("Equalized Image", dst);
        showHistogram("Original Histogram", histogram, 256, 256);
        showHistogram("Equalized Histogram", histogram_eq, 256, 256);
        waitKey();
    }
}