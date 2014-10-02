#include "objectfinder.h"
#include <math.h> 
#define PI 3.14159265

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

ObjectFinder::ObjectFinder()
{
	using boost::property_tree::ptree;
	try {

		ptree pt;
		read_ini("conf/camera.ini", pt);
		Vfov = pt.get<float>("Vfov");
		CamHeight = pt.get<float>("Height");
		CamAngleDev = pt.get<float>("AngleDev");
	}
	catch (...){
		ptree pt;
		pt.put("Height", CamHeight);
		pt.put("AngleDev", CamAngleDev);
		pt.put("Vfov", Vfov);



		write_ini("conf/camera.ini", pt);
	};
}
std::pair<int, double> ObjectFinder::Locate(const HSVColorRange &r, const cv::Mat &frame) {
	cv::Point2f point = LocateOnScreen(r, frame);

	return ConvertPixelToRealWorld(point, cv::Point2i(frame.cols, frame.rows));
}

cv::Point2f ObjectFinder::LocateOnScreen(const HSVColorRange &r, const cv::Mat &frame) {

	cv::Mat	imgOriginal = frame;
	cv::Mat imgHSV;
	//	cv::imshow("Thresholded Image 2", imgOriginal); //show the thresholded image
	cvtColor(imgOriginal, imgHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
	//	cv::imshow("Thresholded Image 3", imgHSV); //show the thresholded image
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
	for (int i = 0; i < contours.size(); i++) // iterate through each contour.
	{
		double a = cv::contourArea(contours[i], false);  //  Find the area of contour
		if (a > largest_area){
			largest_area = a;
			largest_contour_index = i;                //Store the index of largest contour
			bounding_rect = cv::boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
		}
	}
	cv::Scalar color(255, 255, 255);


	//drawContours(dst, contours, largest_contour_index, color, CV_FILLED, 8, hierarchy); // Draw the largest contour using previously stored index.

	//find center
	cv::Scalar colorCircle(133, 33, 55);
	cv::Point2f center;
	if (contours.size() > largest_contour_index){
		cv::Moments M = cv::moments(contours[largest_contour_index]);
		center = cv::Point2f(M.m10 / M.m00, M.m01 / M.m00);

	}



	//Draw circle
	cv::circle(imgOriginal, center, 10, colorCircle, 3);
	cv::imshow("Thresholded Image", imgOriginal); //show the thresholded image
	return center;
}
std::pair<int, double> ObjectFinder::ConvertPixelToRealWorld(const cv::Point2f &point, const cv::Point2i &frame_size)
{
	const cv::Point2d center; (frame_size.x / 2.0, frame_size.y / 2.0);

	//Calculating distance
	float angle = (Vfov * (point.y - center.y) / center.y) + CamAngleDev;
	float distance = CamHeight / tan(angle * PI / 180);
	//Calculating horizontal deviation
	int HorizontalDev = center.x - point.x; //positive value, if left, negative if right compared to center axis

	std::cout << distance << std::endl;


	if (center.y == 0 && center.x == 0){ //If there is no object found
		return std::make_pair(-1, -1);
	}
	else{
		return std::make_pair(HorizontalDev, distance);
	}

	
}
