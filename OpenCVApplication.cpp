// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"
#include <opencv2/core/utils/logger.hpp>
#include "image.h"

wchar_t* projectPath;

void testOpenImage()
{
	char fname[MAX_PATH];
	while(openFileDlg(fname))
	{
		Mat src;
		src = imread(fname);
		imshow("image",src);
		waitKey();
	}
}

void testOpenImagesFld()
{
	char folderName[MAX_PATH];
	if (openFolderDlg(folderName)==0)
		return;
	char fname[MAX_PATH];
	FileGetter fg(folderName,"bmp");
	while(fg.getNextAbsFile(fname))
	{
		Mat src;
		src = imread(fname);
		imshow(fg.getFoundFileName(),src);
		if (waitKey()==27) //ESC pressed
			break;
	}
}

void testImageOpenAndSave()
{
	_wchdir(projectPath);

	Mat src, dst;

	src = imread("Images/Lena_24bits.bmp", IMREAD_COLOR);	// Read the image

	if (!src.data)	// Check for invalid input
	{
		printf("Could not open or find the image\n");
		return;
	}

	// Get the image resolution
	Size src_size = Size(src.cols, src.rows);

	// Display window
	const char* WIN_SRC = "Src"; //window for the source image
	namedWindow(WIN_SRC, WINDOW_AUTOSIZE);
	moveWindow(WIN_SRC, 0, 0);

	const char* WIN_DST = "Dst"; //window for the destination (processed) image
	namedWindow(WIN_DST, WINDOW_AUTOSIZE);
	moveWindow(WIN_DST, src_size.width + 10, 0);

	cvtColor(src, dst, COLOR_BGR2GRAY); //converts the source image to a grayscale one

	imwrite("Images/Lena_24bits_gray.bmp", dst); //writes the destination to file

	imshow(WIN_SRC, src);
	imshow(WIN_DST, dst);

	waitKey(0);
}

void testNegativeImage()
{
	char fname[MAX_PATH];
	while(openFileDlg(fname))
	{
		double t = (double)getTickCount(); // Get the current time [s]
		
		Mat src = imread(fname,IMREAD_GRAYSCALE);
		int height = src.rows;
		int width = src.cols;
		Mat dst = Mat(height,width,CV_8UC1);
		// Accessing individual pixels in an 8 bits/pixel image
		// Inefficient way -> slow
		for (int i=0; i<height; i++)
		{
			for (int j=0; j<width; j++)
			{
				uchar val = src.at<uchar>(i,j);
				uchar neg = 255 - val;
				dst.at<uchar>(i,j) = neg;
			}
		}

		// Get the current time again and compute the time difference [s]
		t = ((double)getTickCount() - t) / getTickFrequency();
		// Print (in the console window) the processing time in [ms] 
		printf("Time = %.3f [ms]\n", t * 1000);

		imshow("input image",src);
		imshow("negative image",dst);
		waitKey();
	}
}

void testNegativeImageFast()
{
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		Mat src = imread(fname, IMREAD_GRAYSCALE);
		int height = src.rows;
		int width = src.cols;
		Mat dst = src.clone();

		double t = (double)getTickCount(); // Get the current time [s]

		// The fastest approach of accessing the pixels -> using pointers
		uchar *lpSrc = src.data;
		uchar *lpDst = dst.data;
		int w = (int) src.step; // no dword alignment is done !!!
		for (int i = 0; i<height; i++)
			for (int j = 0; j < width; j++) {
				uchar val = lpSrc[i*w + j];
				lpDst[i*w + j] = 255 - val;
			}

		// Get the current time again and compute the time difference [s]
		t = ((double)getTickCount() - t) / getTickFrequency();
		// Print (in the console window) the processing time in [ms] 
		printf("Time = %.3f [ms]\n", t * 1000);

		imshow("input image",src);
		imshow("negative image",dst);
		waitKey();
	}
}

void testColor2Gray()
{
	char fname[MAX_PATH];
	while(openFileDlg(fname))
	{
		Mat src = imread(fname);

		int height = src.rows;
		int width = src.cols;

		Mat dst = Mat(height,width,CV_8UC1);

		// Accessing individual pixels in a RGB 24 bits/pixel image
		// Inefficient way -> slow
		for (int i=0; i<height; i++)
		{
			for (int j=0; j<width; j++)
			{
				Vec3b v3 = src.at<Vec3b>(i,j);
				uchar b = v3[0];
				uchar g = v3[1];
				uchar r = v3[2];
				dst.at<uchar>(i,j) = (r+g+b)/3;
			}
		}
		
		imshow("input image",src);
		imshow("gray image",dst);
		waitKey();
	}
}

void testBGR2HSV()
{
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		Mat src = imread(fname);
		int height = src.rows;
		int width = src.cols;

		// HSV components
		Mat H = Mat(height, width, CV_8UC1);
		Mat S = Mat(height, width, CV_8UC1);
		Mat V = Mat(height, width, CV_8UC1);

		// Defining pointers to each matrix (8 bits/pixels) of the individual components H, S, V 
		uchar* lpH = H.data;
		uchar* lpS = S.data;
		uchar* lpV = V.data;

		Mat hsvImg;
		cvtColor(src, hsvImg, COLOR_BGR2HSV);

		// Defining the pointer to the HSV image matrix (24 bits/pixel)
		uchar* hsvDataPtr = hsvImg.data;

		for (int i = 0; i<height; i++)
		{
			for (int j = 0; j<width; j++)
			{
				int hi = i*width * 3 + j * 3;
				int gi = i*width + j;

				lpH[gi] = hsvDataPtr[hi] * 510 / 360;	// lpH = 0 .. 255
				lpS[gi] = hsvDataPtr[hi + 1];			// lpS = 0 .. 255
				lpV[gi] = hsvDataPtr[hi + 2];			// lpV = 0 .. 255
			}
		}

		imshow("input image", src);
		imshow("H", H);
		imshow("S", S);
		imshow("V", V);

		waitKey();
	}
}

void testResize()
{
	char fname[MAX_PATH];
	while(openFileDlg(fname))
	{
		Mat src;
		src = imread(fname);
		Mat dst1,dst2;
		//without interpolation
		resizeImg(src,dst1,320,false);
		//with interpolation
		resizeImg(src,dst2,320,true);
		imshow("input image",src);
		imshow("resized image (without interpolation)",dst1);
		imshow("resized image (with interpolation)",dst2);
		waitKey();
	}
}

void testCanny()
{
	char fname[MAX_PATH];
	while(openFileDlg(fname))
	{
		Mat src,dst,gauss;
		src = imread(fname,IMREAD_GRAYSCALE);
		double k = 0.4;
		int pH = 50;
		int pL = (int) k*pH;
		GaussianBlur(src, gauss, Size(5, 5), 0.8, 0.8);
		Canny(gauss,dst,pL,pH,3);
		imshow("input image",src);
		imshow("canny",dst);
		waitKey();
	}
}

void testVideoSequence()
{
	_wchdir(projectPath);

	VideoCapture cap("Videos/rubic.avi"); // off-line video from file
	//VideoCapture cap(0);	// live video from web cam
	if (!cap.isOpened()) {
		printf("Cannot open video capture device.\n");
		waitKey(0);
		return;
	}
		
	Mat edges;
	Mat frame;
	char c;

	while (cap.read(frame))
	{
		Mat grayFrame;
		cvtColor(frame, grayFrame, COLOR_BGR2GRAY);
		Canny(grayFrame,edges,40,100,3);
		imshow("source", frame);
		imshow("gray", grayFrame);
		imshow("edges", edges);
		c = waitKey(100);  // waits 100ms and advances to the next frame
		if (c == 27) {
			// press ESC to exit
			printf("ESC pressed - capture finished\n"); 
			break;  //ESC pressed
		};
	}
}


void testSnap()
{
	_wchdir(projectPath);

	VideoCapture cap(0); // open the deafult camera (i.e. the built in web cam)
	if (!cap.isOpened()) // openenig the video device failed
	{
		printf("Cannot open video capture device.\n");
		return;
	}

	Mat frame;
	char numberStr[256];
	char fileName[256];
	
	// video resolution
	Size capS = Size((int)cap.get(CAP_PROP_FRAME_WIDTH),
		(int)cap.get(CAP_PROP_FRAME_HEIGHT));

	// Display window
	const char* WIN_SRC = "Src"; //window for the source frame
	namedWindow(WIN_SRC, WINDOW_AUTOSIZE);
	moveWindow(WIN_SRC, 0, 0);

	const char* WIN_DST = "Snapped"; //window for showing the snapped frame
	namedWindow(WIN_DST, WINDOW_AUTOSIZE);
	moveWindow(WIN_DST, capS.width + 10, 0);

	char c;
	int frameNum = -1;
	int frameCount = 0;

	for (;;)
	{
		cap >> frame; // get a new frame from camera
		if (frame.empty())
		{
			printf("End of the video file\n");
			break;
		}

		++frameNum;
		
		imshow(WIN_SRC, frame);

		c = waitKey(10);  // waits a key press to advance to the next frame
		if (c == 27) {
			// press ESC to exit
			printf("ESC pressed - capture finished");
			break;  //ESC pressed
		}
		if (c == 115){ //'s' pressed - snap the image to a file
			frameCount++;
			fileName[0] = NULL;
			sprintf(numberStr, "%d", frameCount);
			strcat(fileName, "Images/A");
			strcat(fileName, numberStr);
			strcat(fileName, ".bmp");
			bool bSuccess = imwrite(fileName, frame);
			if (!bSuccess) 
			{
				printf("Error writing the snapped image\n");
			}
			else
				imshow(WIN_DST, frame);
		}
	}

}

void MyCallBackFunc(int event, int x, int y, int flags, void* param)
{
	//More examples: http://opencvexamples.blogspot.com/2014/01/detect-mouse-clicks-and-moves-on-image.html
	Mat* src = (Mat*)param;
	if (event == EVENT_LBUTTONDOWN)
		{
			printf("Pos(x,y): %d,%d  Color(RGB): %d,%d,%d\n",
				x, y,
				(int)(*src).at<Vec3b>(y, x)[2],
				(int)(*src).at<Vec3b>(y, x)[1],
				(int)(*src).at<Vec3b>(y, x)[0]);
		}
}

void testMouseClick()
{
	Mat src;
	// Read image from file 
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		src = imread(fname);
		//Create a window
		namedWindow("My Window", 1);

		//set the callback function for any mouse event
		setMouseCallback("My Window", MyCallBackFunc, &src);

		//show the image
		imshow("My Window", src);

		// Wait until user press some key
		waitKey(0);
	}
}

/* Histogram display function - display a histogram using bars (simlilar to L3 / Image Processing)
Input:
name - destination (output) window name
hist - pointer to the vector containing the histogram values
hist_cols - no. of bins (elements) in the histogram = histogram image width
hist_height - height of the histogram image
Call example:
showHistogram ("MyHist", hist_dir, 255, 200);
*/
void showHistogram(const std::string& name, int* hist, const int  hist_cols, const int hist_height)
{
	Mat imgHist(hist_height, hist_cols, CV_8UC3, CV_RGB(255, 255, 255)); // constructs a white image

	//computes histogram maximum
	int max_hist = 0;
	for (int i = 0; i<hist_cols; i++)
	if (hist[i] > max_hist)
		max_hist = hist[i];
	double scale = 1.0;
	scale = (double)hist_height / max_hist;
	int baseline = hist_height - 1;

	for (int x = 0; x < hist_cols; x++) {
		Point p1 = Point(x, baseline);
		Point p2 = Point(x, baseline - cvRound(hist[x] * scale));
		line(imgHist, p1, p2, CV_RGB(255, 0, 255)); // histogram bins colored in magenta
	}

	imshow(name, imgHist);
}

void additive_grayscale() {
	Mat_<uchar> image = imread("C:\\Users\\rcm14\\Pictures\\osu_wallpaper.jpg", 0);
	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			image.at<uchar>(i, j) = image.at<uchar>(i, j) + min(255 - image.at<uchar>(i, j), 32);
		}
	}

	cv::imshow("image", image);
	waitKey(0);
}

void multiplicative_grayscale() {
	Mat_<uchar> image = imread("C:\\Users\\rcm14\\Pictures\\osu_wallpaper.jpg", 0);
	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			image.at<uchar>(i, j) = image.at<uchar>(i, j) * 255 / image.at<uchar>(i, j);
		}
	}
	cv::imshow("image", image);
	waitKey(0);
}

void coloured_image() {
	Mat image = Mat(256, 256, CV_8UC3, Scalar(0, 0, 0));

	int cadran = 128;

	for (int i = 0; i < cadran; i++) {
		for (int j = 0; j < cadran; j++) {
			image.at<Vec3b>(i, j) = Vec3b(255, 255, 255);
		}
	}

	for (int i = 0; i < cadran; i++) {
		for (int j = cadran; j < 256; j++) {
			image.at<Vec3b>(i, j) = Vec3b(0, 0, 255);
		}
	}

	for (int i = cadran; i < 256; i++) {
		for (int j = 0; j < cadran; j++) {
			image.at<Vec3b>(i, j) = Vec3b(0, 255, 0);
		}
	}

	for (int i = cadran; i < 256; i++) {
		for (int j = cadran; j < 256; j++) {
			image.at<Vec3b>(i, j) = Vec3b(0, 255, 255);
		}
	}

	imshow("image", image);
	waitKey(0);

	imwrite("coloured_image.png", image);
}

void matrix_inverse() {
	Mat matrix = (Mat_<float>(3, 3) <<
		1.0, 2.0, 3.0,
		4.0, 5.0, 6.0,
		7.0, 8.0, 10.0);
	
	float det = determinant(matrix);
	if (det < 0.00001) return;

	Mat inverse = matrix.inv();
	std::cout << inverse << std::endl << std::endl;
}

void split_channels() {
	Mat image = imread("C:\\Users\\rcm14\\Pictures\\osu_wallpaper.jpg", cv::ImreadModes::IMREAD_COLOR);
	Mat blue_channel = Mat(image.rows, image.cols, CV_8UC3, Scalar(0, 0, 0));
	Mat red_channel = Mat(image.rows, image.cols, CV_8UC3, Scalar(0, 0, 0));
	Mat green_channel = Mat(image.rows, image.cols, CV_8UC3, Scalar(0, 0, 0));

	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			blue_channel.at<Vec3b>(i, j)[0] = image.at<Vec3b>(i, j)[0];
			green_channel.at<Vec3b>(i, j)[1] = image.at<Vec3b>(i, j)[1];
			red_channel.at<Vec3b>(i, j)[2] = image.at<Vec3b>(i, j)[2];
		}
	}

	cv::imshow("Blue channel", blue_channel);
	cv::imshow("Green channel", green_channel);
	cv::imshow("Red channel", red_channel);
	waitKey(0);
}

std::shared_ptr<Mat> rgb24_to_grayscale() {
	Mat image = imread("C:\\Users\\rcm14\\Pictures\\osu_wallpaper.jpg", cv::ImreadModes::IMREAD_COLOR);
	Mat grayscale_image = Mat(image.rows, image.cols, CV_8UC1, Scalar(0));

	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			grayscale_image.at<uchar>(i, j) = (image.at<Vec3b>(i, j)[0] + image.at<Vec3b>(i, j)[1] + image.at<Vec3b>(i, j)[2]) / 3;
		}
	}

	return std::make_shared<Mat>(grayscale_image);
}

void grayscale_to_black_and_white() {
	auto image = rgb24_to_grayscale();

	uint8_t threshold = 0;
	std::cout << "Enter the threshold: ";
	std::cin >> threshold;

	for (int i = 0; i < image->rows; i++) {
		for (int j = 0; j < image->cols; j++) {
			if (image->at<uchar>(i, j) > threshold) {
				image->at<uchar>(i, j) = 255;
			}
			else {
				image->at<uchar>(i, j) = 0;
			}
		}
	}

	cv::imshow("Grayscale", (*image));
	waitKey(0);

}

void rgb24_to_hsv() {
	Mat image = imread("C:\\Users\\rcm14\\Pictures\\test_images\\flowers_24bits.bmp", cv::ImreadModes::IMREAD_COLOR);
	Mat hue = Mat(image.rows, image.cols, CV_8UC1, Scalar(0));
	Mat saturation = Mat(image.rows, image.cols, CV_8UC1, Scalar(0));
	Mat value = Mat(image.rows, image.cols, CV_8UC1, Scalar(0));

	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			float normalized_blue = image.at<Vec3b>(i, j)[0] / 255.0;
			float normalized_green = image.at<Vec3b>(i, j)[1] / 255.0;
			float normalized_red = image.at<Vec3b>(i, j)[2] / 255.0;

			float max_colour = max(max(normalized_blue, normalized_green), normalized_red);
			float min_colour = min(min(normalized_blue, normalized_green), normalized_red);
			float colour_difference = max_colour - min_colour;

			value.at<uchar>(i, j) = max_colour * 255.0;
			if (max_colour != 0) {
				saturation.at<uchar>(i, j) = colour_difference / max_colour * 255.0;
			}
			else {
				saturation.at<uchar>(i, j) = 0;
			}

			if (colour_difference != 0) {
				if (max_colour == normalized_green) {
					hue.at<uchar>(i, j) = 60 * (normalized_green - normalized_blue) / colour_difference;
				}
				else if (max_colour == normalized_red) {
					hue.at<uchar>(i, j) = 120 + 60 * (normalized_blue - normalized_red) / colour_difference;
				}
				else {
					hue.at<uchar>(i, j) = 240 + 60 * (normalized_red - normalized_green) / colour_difference;
				}
			}
			else {
				hue.at<uchar>(i, j) = 0;
			}

			if (hue.at<uchar>(i, j) < 0) {
				hue.at<uchar>(i, j) = hue.at<uchar>(i, j) + 360;
			}

			hue.at<uchar>(i, j) = hue.at<uchar>(i, j) * 255.0 / 360.0;
		}
	}

	imshow("Hue", hue);
	imshow("Saturation", saturation);
	imshow("Value", value);
	waitKey(0);
}

void histogram(char* file_name, int* histogram) {
	Mat_<uchar> grayscale_image = imread(file_name, ImreadModes::IMREAD_GRAYSCALE);
	for (int i = 0; i < grayscale_image.rows; i++) {
		for (int j = 0; j < grayscale_image.cols; j++) {
			histogram[grayscale_image.at<uchar>(i, j)]++;
		}
	}
}

std::vector<int> histogram(const char* file_name) {
	std::vector<int> histogram(256, 0);
	Mat_<uchar> grayscale_image = imread(file_name, ImreadModes::IMREAD_GRAYSCALE);
	for (int i = 0; i < grayscale_image.rows; i++) {
		for (int j = 0; j < grayscale_image.cols; j++) {
			histogram[grayscale_image.at<uchar>(i, j)]++;
		}
	}
	return histogram;
}

std::vector<int> histogram(const char* file_name, int nr_intervals) {
	std::vector<int> hist(nr_intervals, 0);
	Mat_<uchar> grayscale_image = imread(file_name, ImreadModes::IMREAD_GRAYSCALE);

	float interval_size = 256.0f / nr_intervals;

	for (int i = 0; i < grayscale_image.rows; i++) {
		for (int j = 0; j < grayscale_image.cols; j++) {
			int pixel_value = grayscale_image.at<uchar>(i, j);

			int bin = min(int(pixel_value / interval_size), nr_intervals - 1);

			hist[bin]++;
		}
	}

	return hist;
}

std::vector<float> fdp(std::vector<int> histogram, int normalization_value) {
	std::vector<float> fdp(histogram.size(), 0.0f);
	for (int i = 0; i < fdp.size(); i++) {
		fdp[i] = histogram[i] / (float) normalization_value;
	}
	return fdp;
}

void show_histogram(std::vector<int> histogram) {
	int* hist = (int*) malloc(histogram.size() * sizeof(int));
	for (int i = 0; i < histogram.size(); i++) {
		hist[i] = histogram[i];
	}
	showHistogram("histogram", hist, histogram.size(), 256);
	waitKey(0);
	free(hist);
}


std::vector<uchar> find_histogram_peaks(const std::vector<int>& hist) {
	std::vector<uchar> peaks;
	int WH = 5;
	float TH = 0.0003f;


	int total_pixels = 0;
	for (int i = 0; i < 256; i++) {
		total_pixels += hist[i];
	}
	std::vector<float> normalized_hist = fdp(hist, total_pixels);

	for (int k = WH; k < 256 - WH; k++) {
		float sum = 0.0f;
		for (int j = k - WH; j <= k + WH; j++) {
			sum += normalized_hist[j];
		}
		float v = sum / (2 * WH + 1);

		bool is_peak = (normalized_hist[k] > v + TH);
		for (int j = k - WH; j <= k + WH; j++) {
			if (normalized_hist[j] > normalized_hist[k]) {
				is_peak = false;
				break;
			}
		}

		if (is_peak) {
			peaks.push_back(k);
		}
	}

	peaks.insert(peaks.begin(), 0);
	peaks.push_back(255);

	return peaks;
}

uchar find_closest_peak(const std::vector<uchar>& peaks, uchar value) {
	uchar closest = peaks[0];
	int min_distance = 255;

	for (uchar peak : peaks) {
		int distance = std::abs(static_cast<int>(value) - static_cast<int>(peak));
		if (distance < min_distance) {
			min_distance = distance;
			closest = peak;
		}
	}

	return closest;
}

void multiple_threshold(const char* file_name) {
	Mat_<uchar> grayscale_image = imread(file_name, ImreadModes::IMREAD_GRAYSCALE);

	std::vector<int> hist = histogram(file_name);

	std::vector<uchar> peaks = find_histogram_peaks(hist);

	Mat_<uchar> result = grayscale_image.clone();

	for (int i = 0; i < grayscale_image.rows; i++) {
		for (int j = 0; j < grayscale_image.cols; j++) {
			uchar old_pixel = grayscale_image.at<uchar>(i, j);
			uchar new_pixel = find_closest_peak(peaks, old_pixel);
			result.at<uchar>(i, j) = new_pixel;
		}
	}

	imshow("Multiple Threshold", result);
	waitKey(0);
}

void floyd_steinberg(char* file_name) {
	Mat_<uchar> grayscale_image = imread(file_name, ImreadModes::IMREAD_GRAYSCALE);
	std::vector<int> hist = histogram(file_name);

	std::vector<uchar> output_values = find_histogram_peaks(hist);

	for (int i = 0; i < grayscale_image.rows; i++) {
		for (int j = 0; j < grayscale_image.cols; j++) {
			uchar old_pixel = grayscale_image.at<uchar>(i, j);
			uchar new_pixel = find_closest_peak(output_values, old_pixel);
			grayscale_image.at<uchar>(i, j) = new_pixel;
			int error = old_pixel - new_pixel;
			if (i < grayscale_image.rows && j + 1 < grayscale_image.cols) {
				grayscale_image.at<uchar>(i, j + 1) = grayscale_image.at<uchar>(i, j + 1) + 7 * error / 16;
			}
			if (i + 1 < grayscale_image.rows && j - 1 >= 0) {
				grayscale_image.at<uchar>(i + 1, j - 1) = grayscale_image.at<uchar>(i + 1, j - 1) + 3 * error / 16;
			}
			if (i + 1 < grayscale_image.rows && j < grayscale_image.cols) {
				grayscale_image.at<uchar>(i + 1, j) = grayscale_image.at<uchar>(i + 1, j) + 5 * error / 16;
			}
			if (i + 1 < grayscale_image.rows && j + 1 < grayscale_image.cols) {
				grayscale_image.at<uchar>(i + 1, j + 1) = grayscale_image.at<uchar>(i + 1, j + 1) + error / 16;
			}
			
		}
	}
	imshow("floyd_steinberg", grayscale_image);
	waitKey(0);
}




int main() 
{
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_FATAL);
    projectPath = _wgetcwd(0, 0);

	int op;
	do
	{
		system("cls");
		destroyAllWindows();
		printf("Menu:\n");
		printf(" 1 - Open image\n");
		printf(" 2 - Open BMP images from folder\n");
		printf(" 3 - Image negative\n");
		printf(" 4 - Image negative (fast)\n");
		printf(" 5 - BGR->Gray\n");
		printf(" 6 - BGR->Gray (fast, save result to disk) \n");
		printf(" 7 - BGR->HSV\n");
		printf(" 8 - Resize image\n");
		printf(" 9 - Canny edge detection\n");
		printf(" 10 - Edges in a video sequence\n");
		printf(" 11 - Snap frame from live video\n");
		printf(" 12 - Mouse callback demo\n");
		printf(" 13 - Additive grayscale\n");
		printf(" 14 - Multiplicative grayscale\n");
		printf(" 15 - Coloured image\n");
		printf(" 16 - Matrix inverse\n");
		printf(" 17 - Channel separation\n");
		printf(" 18 - RGB24 to grayscale image\n");
		printf(" 19 - Grayscale to white and black\n");
		printf(" 20 - HSV image\n");
		printf(" 21 - Show histogram\n");
		printf(" 22 - Show split histogram\n");
		printf(" 23 - Multiple thresholds\n");
		printf(" 24 - Floyd-Steinberg\n");
		printf(" 25 - Select object and analyze properties\n");
		printf(" 26 - Filter objects by area and orientation\n");
		printf(" 0 - Exit\n\n");
		printf("Option: ");
		scanf("%d",&op);
		switch (op)
		{
			case 1:
				testOpenImage();
				break;
			case 2:
				testOpenImagesFld();
				break;
			case 3:
				testNegativeImage();
				break;
			case 4:
				testNegativeImageFast();
				break;
			case 5:
				testColor2Gray();
				break;
			case 6:
				testImageOpenAndSave();
				break;
			case 7:
				testBGR2HSV();
				break;
			case 8:
				testResize();
				break;
			case 9:
				testCanny();
				break;
			case 10:
				testVideoSequence();
				break;
			case 11:
				testSnap();
				break;
			case 12:
				testMouseClick();
				break;
			case 13:
				additive_grayscale();
				break;
			case 14:
				multiplicative_grayscale();
				break;
			case 15:
				coloured_image();
				break;
			case 16:
				matrix_inverse();
				break;
			case 17:
				split_channels();
				break;
			case 18:
				rgb24_to_grayscale();
				break;
			case 19:
				grayscale_to_black_and_white();
				break;
			case 20:
				rgb24_to_hsv();
				break;
			case 21:
				show_histogram(histogram("C:\\Users\\rcm14\\Pictures\\lab3\\pout.bmp"));
				break;
			case 22:
				show_histogram(histogram("C:\\Users\\rcm14\\Pictures\\lab3\\pout.bmp", 120));
				break;
			case 23:
				multiple_threshold("C:\\Users\\rcm14\\Pictures\\lab3\\pout.bmp");
				break;
			case 24:
				floyd_steinberg("C:\\Users\\rcm14\\Pictures\\lab3\\pout.bmp");
				break;
			case 25:
				selectObjectAndAnalyze();
				break;
			case 26:
				filterObjectsByAreaAndOrientation();
				break;
		}
	}
	while (op!=0);
	return 0;
}