#include "stdafx.h"
#include "image.h"
#include "common.h"

ObjectProps computeObjectProperties(const Mat& labeledImg, int label) {
    ObjectProps props;
    props.area = 0;
    props.contour.clear();

    int sumX = 0, sumY = 0;

    float m11 = 0, m20 = 0, m02 = 0;

    vector<Point> objectPixels;
    for (int i = 0; i < labeledImg.rows; i++) {
        for (int j = 0; j < labeledImg.cols; j++) {
            if (labeledImg.at<uchar>(i, j) == label) {
                objectPixels.push_back(Point(j, i));
                props.area++;
                sumX += j;
                sumY += i;
            }
        }
    }

    props.center = Point2f(sumX / (float)props.area, sumY / (float)props.area);

    for (const auto& p : objectPixels) {
        float dx = p.x - props.center.x;
        float dy = p.y - props.center.y;
        m11 += dx * dy;
        m20 += dx * dx;
        m02 += dy * dy;
    }

    m11 /= props.area;
    m20 /= props.area;
    m02 /= props.area;

    props.orientation = 0.5 * atan2(2 * m11, m20 - m02) * 180 / CV_PI;

    double lambda1 = 0.5 * (m20 + m02 + sqrt(4 * m11 * m11 + (m20 - m02) * (m20 - m02)));
    double lambda2 = 0.5 * (m20 + m02 - sqrt(4 * m11 * m11 + (m20 - m02) * (m20 - m02)));
    props.elongation = sqrt(lambda1 / max(lambda2, 1e-6));

    vector<vector<Point>> contours;
    Mat binary = Mat::zeros(labeledImg.size(), CV_8UC1);
    for (const auto& p : objectPixels) {
        binary.at<uchar>(p.y, p.x) = 255;
    }
    findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

    if (!contours.empty()) {
        props.contour = contours[0];
        props.perimeter = props.contour.size();
    }
    else {
        props.perimeter = 0;
    }

    props.thinnessFactor = 4 * CV_PI * props.area / (props.perimeter * props.perimeter + 1e-6);

    return props;
}

void computeProjections(const Mat& labeledImg, int label, Mat& horizontalProjection, Mat& verticalProjection) {
    int width = labeledImg.cols;
    int height = labeledImg.rows;

    horizontalProjection = Mat::zeros(1, width, CV_32SC1);
    verticalProjection = Mat::zeros(height, 1, CV_32SC1);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (labeledImg.at<uchar>(i, j) == label) {
                horizontalProjection.at<int>(0, j)++;
                verticalProjection.at<int>(i, 0)++;
            }
        }
    }
}

void selectObjectAndAnalyze() {
    char fname[MAX_PATH];
    while (openFileDlg(fname)) {
        Mat labeledImg = imread(fname, IMREAD_GRAYSCALE);
        if (labeledImg.empty()) {
            printf("Could not open or find the labeled image\n");
            continue;
        }

        Mat displayContourImg = Mat::zeros(labeledImg.size(), CV_8UC3);
        Mat displayProjectionImg = Mat::zeros(labeledImg.size(), CV_8UC3);
        cv::cvtColor(labeledImg, displayContourImg, COLOR_GRAY2BGR);
        cv::cvtColor(labeledImg, displayProjectionImg, COLOR_GRAY2BGR);

        imshow("Labeled Image - Click on an object", labeledImg);

        struct ClickData {
            Mat labeledImg;
            Mat displayContourImg;
            Mat displayProjectionImg;
            bool clicked;
        };

        ClickData clickData = { labeledImg, displayContourImg, displayProjectionImg, false };

        auto clickCallback = [](int event, int x, int y, int flags, void* userdata) {
            ClickData* data = static_cast<ClickData*>(userdata);
            if (event == EVENT_LBUTTONDOWN) {
                data->clicked = true;

                int label = data->labeledImg.at<uchar>(y, x);

                if (label == 0) {
                    printf("Clicked on background (label 0), please click on an object\n");
                    return;
                }

                ObjectProps props = computeObjectProperties(data->labeledImg, label);

                printf("Object Properties (Label %d):\n", label);
                printf("Area: %d pixels\n", props.area);
                printf("Center of Mass: (%.2f, %.2f)\n", props.center.x, props.center.y);
                printf("Orientation: %.2f degrees\n", props.orientation);
                printf("Perimeter: %d pixels\n", props.perimeter);
                printf("Thinness Factor: %.4f\n", props.thinnessFactor);
                printf("Elongation (Aspect Ratio): %.4f\n", props.elongation);

                cv::cvtColor(data->labeledImg, data->displayContourImg, COLOR_GRAY2BGR);
                cv::cvtColor(data->labeledImg, data->displayProjectionImg, COLOR_GRAY2BGR);

                vector<vector<Point>> contours;
                contours.push_back(props.contour);
                drawContours(data->displayContourImg, contours, 0, Scalar(0, 0, 255), 1);

                circle(data->displayContourImg, Point(props.center.x, props.center.y), 3, Scalar(0, 255, 0), -1);

                float length = 50;
                float radians = props.orientation * CV_PI / 180.0;
                Point p1(props.center.x - length * cos(radians), props.center.y - length * sin(radians));
                Point p2(props.center.x + length * cos(radians), props.center.y + length * sin(radians));
                line(data->displayContourImg, p1, p2, Scalar(255, 0, 0), 2);

                Mat horizontalProj, verticalProj;
                computeProjections(data->labeledImg, label, horizontalProj, verticalProj);

                double maxHProj, maxVProj;
                minMaxLoc(horizontalProj, nullptr, &maxHProj);
                minMaxLoc(verticalProj, nullptr, &maxVProj);

                for (int j = 0; j < data->labeledImg.cols; j++) {
                    if (horizontalProj.at<int>(0, j) > 0) {
                        int height = (horizontalProj.at<int>(0, j) * 100) / (maxHProj + 1);
                        line(data->displayProjectionImg,
                            Point(j, data->labeledImg.rows - 1),
                            Point(j, data->labeledImg.rows - 1 - height),
                            Scalar(0, 255, 0), 1);
                    }
                }

                for (int i = 0; i < data->labeledImg.rows; i++) {
                    if (verticalProj.at<int>(i, 0) > 0) {
                        int width = (verticalProj.at<int>(i, 0) * 100) / (maxVProj + 1);
                        line(data->displayProjectionImg,
                            Point(0, i),
                            Point(width, i),
                            Scalar(0, 0, 255), 1);
                    }
                }

                imshow("Object Contour and Features", data->displayContourImg);
                imshow("Object Projections", data->displayProjectionImg);
            }
            };

        setMouseCallback("Labeled Image - Click on an object", clickCallback, &clickData);

        waitKey(0);
        destroyAllWindows();
    }
}

void filterObjectsByAreaAndOrientation() {
    char fname[MAX_PATH];
    while (openFileDlg(fname)) {
        Mat labeledImg = imread(fname, IMREAD_GRAYSCALE);
        if (labeledImg.empty()) {
            printf("Could not open or find the labeled image\n");
            continue;
        }

        int TH_area;
        float phi_LOW, phi_HIGH;

        printf("Enter area threshold (TH_area): ");
        scanf("%d", &TH_area);

        printf("Enter minimum orientation angle (phi_LOW in degrees): ");
        scanf("%f", &phi_LOW);

        printf("Enter maximum orientation angle (phi_HIGH in degrees): ");
        scanf("%f", &phi_HIGH);

        set<int> uniqueLabels;
        for (int i = 0; i < labeledImg.rows; i++) {
            for (int j = 0; j < labeledImg.cols; j++) {
                int label = labeledImg.at<uchar>(i, j);
                if (label > 0) {
                    uniqueLabels.insert(label);
                }
            }
        }

        Mat filteredImg = Mat::zeros(labeledImg.size(), CV_8UC1);

        for (int label : uniqueLabels) {
            ObjectProps props = computeObjectProperties(labeledImg, label);

            float phi = props.orientation;
            if (phi < 0) phi += 180;

            bool meetsAreaCriteria = props.area < TH_area;
            bool meetsOrientationCriteria = (phi >= phi_LOW && phi <= phi_HIGH);

            if (meetsAreaCriteria && meetsOrientationCriteria) {
                for (int i = 0; i < labeledImg.rows; i++) {
                    for (int j = 0; j < labeledImg.cols; j++) {
                        if (labeledImg.at<uchar>(i, j) == label) {
                            filteredImg.at<uchar>(i, j) = label;
                        }
                    }
                }

                printf("Object %d meets criteria (Area: %d, Orientation: %.2f)\n",
                    label, props.area, phi);
            }
            else {
                printf("Object %d doesn't meet criteria (Area: %d, Orientation: %.2f)\n",
                    label, props.area, phi);
            }
        }

        imshow("Original Labeled Image", labeledImg);
        imshow("Filtered Objects", filteredImg);

        waitKey(0);
        destroyAllWindows();
    }
}