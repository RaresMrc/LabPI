#include "labeling.h"
#include "stdafx.h"
#include "common.h"

using namespace std;
using namespace cv;

Mat_<Vec3b> labelImageColoring(const Mat_<uchar>& labeledImg) {
	Mat_<Vec3b> coloredImg = Mat_<Vec3b>::zeros(labeledImg.size());
	const uint MAX_LABEL = 255;
	Vec3b colors[MAX_LABEL] = { 0 };
	for (int i = 0; i < MAX_LABEL; i++) {
		colors[i] = Vec3b(rand() % 256, rand() % 256, rand() % 256);
	}

	const Vec3b backgroundColor = Vec3b(rand() % 256, rand() % 256, rand() % 256);
	for (int i = 0; i < labeledImg.rows; i++) {
		for (int j = 0; j < labeledImg.cols; j++) {
			if (labeledImg(i, j) > 0 && labeledImg(i, j) < MAX_LABEL) {
				coloredImg(i, j) = colors[labeledImg(i, j)];
			}
			else {
				coloredImg(i, j) = backgroundColor;
			}
		}
	}

	imshow("Colored Image", coloredImg);
	waitKey(0);
	return coloredImg;
}

Mat_<uchar> labelImageLateralTraversal() {
	char fname[MAX_PATH];
    Mat image;
    while (openFileDlg(fname)) {
        image = imread(fname, IMREAD_GRAYSCALE);
        if (image.empty()) {
            printf("Could not open or find the image\n");
            continue;
        }
		break;
	}

    Mat_<uchar> labeledImg = Mat_<uchar>::zeros(image.size());

    int label = 0;

    for (int i = 0; i < labeledImg.rows; i++) {
        for (int j = 0; j < labeledImg.cols; j++) {
            if (image.at<uchar>(i, j) == 0 && labeledImg.at<uchar>(i, j) == 0) {
                label++;
                queue<Point> queue;
                labeledImg.at<uchar>(i, j) = label;
                queue.push(Point(i, j));
                while (!queue.empty()) {
					Point p = queue.front();
					queue.pop();
					for (int k = -1; k <= 1; k++) {
						for (int l = -1; l <= 1; l++) {
							if (!(k == 0 && l == 0) && p.x + k >= 0 && p.x + k < labeledImg.rows && p.y + l >= 0 && p.y + l < labeledImg.cols &&
								image.at<uchar>(p.x + k, p.y + l) == 0 && labeledImg.at<uchar>(p.x + k, p.y + l) == 0) {
								labeledImg.at<uchar>(p.x + k, p.y + l) = label;
								queue.push(Point(p.x + k, p.y + l));
							}
						}
					}
                }
            }
        }
    }
	imshow("Initial image", image);
	imshow("Labeled image", labelImageColoring(labeledImg));
    return labeledImg;
}

Mat_<uchar> labelImageTwoPass() {
	char fname[MAX_PATH];
    Mat image;
    while (openFileDlg(fname)) {
        image = imread(fname, IMREAD_GRAYSCALE);
        if (image.empty()) {
            printf("Could not open or find the image\n");
            continue;
        }
		break;
    }

    Mat_<uchar> labeledImg = Mat_<uchar>::zeros(image.size());
    uchar label = 0;
    vector<vector<int>> edges(1000);
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            if (image.at<uchar>(i, j) == 0 && labeledImg.at<uchar>(i, j) == 0) {
                vector<uchar> labels;
				for (int k = -1; k <= 1; k++) {
					for (int l = -1; l <= 1; l++) {
						if (!(k == 0 && l == 0) && i + k >= 0 && i + k < image.rows && j + l >= 0 && j + l < image.cols) {
							if (labeledImg.at<uchar>(i + k, j + l) > 0) {
								labels.push_back(labeledImg.at<uchar>(i + k, j + l));
							}
						}
					}
				}
                if (labels.empty()) {
                    label++;
                    labeledImg.at<uchar>(i, j) = label;
                }
                else {
					uchar x = *min_element(labels.begin(), labels.end());
					labeledImg.at<uchar>(i, j) = x;
					for (int k = -1; k <= 1; k++) {
						for (int l = -1; l <= 1; l++) {
							if (!(k == 0 && l == 0) && i + k >= 0 && i + k < image.rows && j + l >= 0 && j + l < image.cols) {
								if (labeledImg.at<uchar>(i + k, j + l) > 0) {
									uchar y = labeledImg.at<uchar>(i + k, j + l);
									if (x != y) {
										edges[x].push_back(y);
                                        edges[y].push_back(x);
									}
								}
							}
						}
					}
				}
            }
        }
    }

	imshow("After first pass", labelImageColoring(labeledImg));

    uchar newLabel = 0;
	vector<uchar> equivalences(label + 1, 0);
	for (int i = 1; i <= label; i++) {
		if (equivalences[i] == 0) {
			newLabel++;
			queue<uchar> queue;
			queue.push(i);
			equivalences[i] = newLabel;
			while (!queue.empty()) {
				uchar x = queue.front();
				queue.pop();
				for (int y : edges[x]) {
					if (equivalences[y] == 0) {
						equivalences[y] = newLabel;
						queue.push(y);
					}
				}
			}
		}
	}
	for (int i = 0; i < labeledImg.rows; i++) {
		for (int j = 0; j < labeledImg.cols; j++) {
			if (labeledImg.at<uchar>(i, j) > 0) {
				labeledImg.at<uchar>(i, j) = equivalences[labeledImg.at<uchar>(i, j)];
			}
		}
	}
	imshow("Initial image", image);
	imshow("Labeled image", labelImageColoring(labeledImg));
	return labeledImg;
}