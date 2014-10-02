#include "objectfinder.h"
#include <math.h> 
#define PI 3.14159265

std::pair<int, double> ObjectFinder::Locate(const HSVColorRange &r) {

	cv::Mat	imgOriginal = m_pCamera->Capture();
	cv::Mat imgHSV;

	cvtColor(imgOriginal, imgHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

	cv::Mat imgThresholded;

	inRange(imgHSV, cv::Scalar(r.hue.low, r.sat.low, r.val.low), cv::Scalar(r.hue.high, r.sat.high, r.val.high), imgThresholded); //Threshold the image
	cv::Mat dst(imgThresholded.rows, imgThresholded.cols, CV_8U, cv::Scalar::all(0));


	//biggest area
	std::vector<std::vector<cv::Point> > contours; // Vector for storing contour
	std::vector<cv::Vec4i> hierarchy;
	int largest_area = 0;
	int largest_contour_index = 0;
	cv::Rect bounding_rect;
	findContours(imgThresholded, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE); // Find the contours in the image
	for (int i = 0; i< contours.size(); i++) // iterate through each contour.
	{
		double a = cv::contourArea(contours[i], false);  //  Find the area of contour
		if (a>largest_area){
			largest_area = a;
			largest_contour_index = i;                //Store the index of largest contour
			bounding_rect = cv::boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
		}
	}
	cv::Scalar color(255, 255, 255);

	drawContours(dst, contours, largest_contour_index, color, CV_FILLED, 8, hierarchy); // Draw the largest contour using previously stored index.

	//find center
	cv::Scalar colorCircle(133, 33, 55);
	cv::Point2f center;
	if (contours.size() > largest_contour_index){
		cv::Moments M = cv::moments(contours[largest_contour_index]);
		center = cv::Point2f(M.m10 / M.m00, M.m01 / M.m00);
		
	}



	//Draw circle
	cv::circle(dst, center, 10, colorCircle, 3);
	cv::imshow("Thresholded Image", dst); //show the thresholded image


	int frameHeight = dst.rows/2; //half of frame height in pixels
	int frameWidth = dst.cols / 2; //half of frame width in pixels
	
	//Calculating distance
	float angle = (Vfov * (center.y - frameHeight) / frameHeight)+CamAngleDev;
	float distance = CamHeight / tan(angle * PI / 180);
	//Calculating horizontal deviation
	int HorizontalDev = frameWidth - center.x; //positive value, if left, negative if right compared to center axis

	if (center.y == 0 && center.x == 0){ //If there is no object found
		return std::make_pair(-1, -1);
	}
	else{
		return std::make_pair(HorizontalDev, distance);
	}

	
}
