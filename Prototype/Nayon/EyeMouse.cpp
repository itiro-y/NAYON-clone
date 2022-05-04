#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/core/ocl.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

using namespace std;
using namespace cv;

void findCircle(double tLow, double tHigh);
void findCenterCoord(Point &location);
void convertToString(unsigned int &thresh_low, unsigned int &thresh_high, unsigned int &rad_low, unsigned int &rad_high);
void move_it(int x, int y);

Mat frame, grayScale, imCanny;

vector<Vec3f> circles;
vector<Rect> faces;

int xCoord, yCoord;

unsigned int thresh_low = 110;
unsigned int thresh_high = 60;
unsigned int radius_min = 10;
unsigned int radius_max = 40;

String thresh_low_string, thresh_high_string, radius_min_string, radius_max_string;

int main() {

	// Create a VideoCapture object and open the input file
	VideoCapture cap(0);

	// Check if camera opened successfully
	if (!cap.isOpened()) {
		cout << "Error opening camera" << endl;
		return -1;
	}

	//Create window "video"
	namedWindow("Video", WINDOW_AUTOSIZE);

	while (1) {

		//Passing image to a frame
		cap >> frame;

		//If frame is empty, break loop
		if (frame.empty())
			break;

		//grayScale
		//(inputArray, outputArray, mode)
		cvtColor(frame, grayScale, COLOR_BGR2GRAY);

		//Apply gaussian blur
		//(inputArray, outputArray, ksize, sigmaX, sigmaY)
		GaussianBlur(grayScale, grayScale, Size(9, 9), 2, 2);

		//Apply HoughCircle
		findCircle(thresh_low, thresh_high);

		move_it(xCoord, yCoord);

		//Display x and y coordinates
		cout << "x coordinates is: " << xCoord << " and the y coordinate is " << yCoord << endl;

		//Convert unsigned int to string
		convertToString(thresh_low, thresh_high, radius_min, radius_max);

		////Thresh_high string
		putText(imCanny, "Lower threshold: " + thresh_low_string + ". Press \" . \" to increase and  \" , \" to decrease", Point(30, 75), FONT_ITALIC, 0.5, Scalar(255));

		//Thresh_high string
		putText(imCanny, "Higher threshold: " + thresh_high_string + ". Press \" \' \" to increase and  \" ; \" to decrease", Point(30, 50), FONT_ITALIC, 0.5, Scalar(255));

		//radius_min string
		putText(imCanny, "Min radius: " + radius_min_string + ". Press \" ] \" to increase and  \" [ \" to decrease", Point(30, 125), FONT_ITALIC, 0.5, Scalar(255));

		//radius_max string
		putText(imCanny, "Max radius: " + radius_max_string + ". Press \" = \" to increase and  \" - \" to decrease", Point(30, 100), FONT_ITALIC, 0.5, Scalar(255));

		// Display the resulting frame
		imshow("Video", imCanny);

		// Press  ESC on keyboard to exit
		char c = (char)waitKey(25);

		//While running, press
		switch (c) {
			// "ESC" to exit the loop
		case 27: goto exit_loop; break;

			// "'" to increment the high threshold
		case 39: thresh_high += 5; break;

			// ";" to decrement the high threshold
		case 59: thresh_high -= 5; break;

			// "." to increment the low threshold
		case 46: thresh_low += 5; break;

			// "," to decrement the low threshold
		case 44: thresh_low -= 5; break;

			// "]" to increment the min radius
		case 93: radius_min += 5; break;

			// "[" to decrement the min radius
		case 91: radius_min -= 5; break;

			// "=" to increment the max radius
		case 61: radius_max += 5; break;

			//"-" to decrement the max radius
		case 45: radius_max -= 5; break;

		}
	}

	//goto end
exit_loop:;

	// When everything done, release the video capture object
	cap.release();

	// Closes all the frames
	destroyAllWindows();

	return 0;
}

void findCircle(double tLow, double tHigh) {

	//Apply Canny Edge Detection
	Canny(grayScale, imCanny, thresh_low, thresh_high);

	//Apply Hough Circles Transform
	HoughCircles(imCanny, circles, HOUGH_GRADIENT, 2, imCanny.rows / 32, tLow, tHigh, radius_min, radius_max);

	for (size_t i = 0; i < circles.size(); i++) {
		//x-center, y-center, radius
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);

		//Get x and y coordinates
		findCenterCoord(center);

		// Draw the outer circle
		circle(imCanny, center, radius, Scalar(255, 255, 255), 2);

		// Draw the center of the circle
		circle(imCanny, center, 2, Scalar(255, 255, 255), 3);
	}

}
void findCenterCoord(Point &location) {
	xCoord = location.x;
	yCoord = location.y;
}

void convertToString(unsigned int &string_low, unsigned int &string_high, unsigned int &rad_low, unsigned int &rad_high) {
	thresh_low_string = to_string(string_low);
	thresh_high_string = to_string(string_high);
	radius_min_string = to_string(rad_low);
	radius_max_string = to_string(rad_high);
}

void move_it(int x, int y)
{
	//Get screen dimnesions
	double fScreenWidth = ::GetSystemMetrics(SM_CXSCREEN) - 1;
	double fScreenHeight = ::GetSystemMetrics(SM_CYSCREEN) - 1;

	//callibrate coordinates range
	double xx = x * (65535.0f / fScreenWidth);
	double yy = y * (65535.0f / fScreenHeight);

	//initializing
	INPUT  Input = { 0 };
	Input.type = INPUT_MOUSE;

	//movement status|coordinates are normalized and absolute so that the 
	//movement is not done relative to last position; take off absolute specification for relative mode
	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

	//absolute positions 
	Input.mi.dx = xx;
	Input.mi.dy = yy;

	//mouse motions with inputs (number of structs, array of INPUT structures, size of INPUT structure) 
	//function failure without correct 3rd argumment
	//first argument 1 because synthesizing one movement only each call
	::SendInput(1, &Input, sizeof(INPUT));
}
