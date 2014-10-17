#include "objectfinder.h"

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
cv::Point3d ObjectFinder::Locate(const HSVColorRange &r, const cv::Mat &frame) {
	cv::Point2f point = LocateOnScreen(r, frame);
	cv::Point3d info = ConvertPixelToRealWorld(point, cv::Point2i(frame.cols, frame.rows));
	WriteInfoOnScreen(info);
	return info;
}

cv::Point2f ObjectFinder::LocateOnScreen(const HSVColorRange &r, const cv::Mat &frame) {

	cv::Point2f center;
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
	
	cv::Scalar color(255, 255, 255);

	for (int i = 0; i < contours.size(); i++) // iterate through each contour.
	{
		double a = cv::contourArea(contours[i], false);  //  Find the area of contour
		if (a > largest_area){
			largest_area = a;
			largest_contour_index = i;                //Store the index of largest contour
			bounding_rect = cv::boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
		}
		drawContours(imgOriginal, contours, i, color, 1, 8, hierarchy); // Draw the largest contour using previously stored index.
	}


	//find center
	cv::Scalar colorCircle(133, 33, 55);

	if (contours.size() > largest_contour_index){
		cv::Moments M = cv::moments(contours[largest_contour_index]);
		center = cv::Point2f(M.m10 / M.m00, M.m01 / M.m00);
	}

	//Draw circle
	cv::circle(imgOriginal, center, 10, colorCircle, 3);
	cv::imshow("Original", imgOriginal);
	cv::moveWindow("Original", 0, 0);
	return center;
}

cv::Point3d ObjectFinder::ConvertPixelToRealWorld(const cv::Point2f &point, const cv::Point2i &frame_size)
{
	if (point.y < 1 && point.x < 1 || (point.y != point.y) || (point.x != point.x)){ //If there is no object found
		return cv::Point3d(-1, -1, -1);
	}


	const cv::Point2d center (frame_size.x / 2.0, frame_size.y / 2.0);
	//Calculating distance
	double angle = (Vfov * (point.y - center.y) / center.y) + CamAngleDev;
	double distance = CamHeight / tan(angle * PI / 180);
	//Calculating horizontal deviation
	double hor_space = tan(Vfov)*distance;
	double HorizontalDev = (hor_space * (point.x - center.x) / center.x);
	double Hor_angle = atan(HorizontalDev / distance)* 180/PI;
	if (Hor_angle < 0){
		Hor_angle = 360 + Hor_angle;
	}
	return cv::Point3d(distance, HorizontalDev, Hor_angle);
}
void ObjectFinder::WriteInfoOnScreen(const cv::Point3d &info){
	cv::Mat infoWindow(100, 250, CV_8UC3, cv::Scalar::all(0));
	std::ostringstream oss;
	oss << "Distance :" << info.x;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	oss.str("");
	oss << "Horizontal Dev :" << info.y;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 50), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	oss.str("");
	oss << "Horizontal angle :" << info.z;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 80), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	cv::namedWindow("Info Window");
	cv::imshow("Info Window", infoWindow);
	return;
}
