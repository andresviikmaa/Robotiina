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
cv::Point3d ObjectFinder::Locate(const HSVColorRange &r, cv::Mat &frameHSV, cv::Mat &frameBGR, bool gate) {
	cv::Point2f point = LocateOnScreen(r, frameHSV, frameBGR, gate);
	cv::Point3d info = ConvertPixelToRealWorld(point, cv::Point2i(frameHSV.cols, frameHSV.rows));
	WriteInfoOnScreen(info);
	return info;
}

cv::Point2f ObjectFinder::LocateOnScreen(const HSVColorRange &r, cv::Mat &frameHSV, cv::Mat &frameBGR, bool gate) {

	cv::Point2f center;
	cv::Mat imgThresholded;

	inRange(frameHSV, cv::Scalar(r.hue.low, r.sat.low, r.val.low), cv::Scalar(r.hue.high, r.sat.high, r.val.high), imgThresholded); //Threshold the image

	cv::Mat dst(imgThresholded.rows, imgThresholded.cols, CV_8U, cv::Scalar::all(0));

	//biggest area
	std::vector<std::vector<cv::Point> > contours; // Vector for storing contour
	std::vector<cv::Vec4i> hierarchy;
	int largest_area = 0;
	int largest_contour_index = 0;
	cv::Rect bounding_rect;

	findContours(imgThresholded, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // Find the contours in the image
	
	cv::Scalar color(255, 255, 255);
	cv::Scalar color2(0, 0, 255);

	for (int i = 0; i < contours.size(); i++) // iterate through each contour.
	{
		double a = cv::contourArea(contours[i], false);  //  Find the area of contour
		if (a > largest_area){
			largest_area = a;
			largest_contour_index = i;                //Store the index of largest contour
			bounding_rect = cv::boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
		}
		bounding_rect = cv::boundingRect(contours[i]);
		cv::rectangle(frameBGR, bounding_rect, color2);
		drawContours(frameBGR, contours, i, color, 1, 8, hierarchy); // Draw the largest contour using previously stored index.
	}


	//find center
	cv::Scalar colorCircle(133, 33, 55);

	if (contours.size() > largest_contour_index){
		cv::Moments M = cv::moments(contours[largest_contour_index]);
		if (gate){
			bounding_rect = cv::boundingRect(contours[largest_contour_index]);
			double y = bounding_rect.y + bounding_rect.height;
			center = cv::Point2f(M.m10 / M.m00, y);
		}
		else{
			center = cv::Point2f(M.m10 / M.m00, M.m01 / M.m00);
		}
		
	}

	//Draw circle
	cv::circle(frameBGR, center, 10, colorCircle, 3);
	return center;
}

void ObjectFinder::IsolateField(const HSVColorRange &inner, const HSVColorRange &outer, cv::Mat &frameHSV, cv::Mat &frameBGR) {
	//	cv::imshow("Thresholded Image 3", imgHSV); //show the thresholded image
	cv::Mat innerThresholded;
	inRange(frameHSV, cv::Scalar(inner.hue.low, inner.sat.low, inner.val.low), cv::Scalar(inner.hue.high, inner.sat.high, inner.val.high), innerThresholded); //Threshold the image
	cv::Mat outerThresholded;
	inRange(frameHSV, cv::Scalar(outer.hue.low, outer.sat.low, outer.val.low), cv::Scalar(outer.hue.high, outer.sat.high, outer.val.high), outerThresholded); //Threshold the image

	std::vector<std::vector<cv::Point> > contours; // Vector for storing contour
	std::vector<cv::Vec4i> hierarchy;
	std::vector<std::vector<cv::Point> > contours2; // Vector for storing contour
	std::vector<cv::Vec4i> hierarchy2;
#ifdef USE_CONTOURS
	innerThresholded = outerThresholded + innerThresholded;
	findContours(innerThresholded, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // Find the contours in the image
	//findContours(outerThresholded, contours2, hierarchy2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // Find the contours in the image

	cv::Scalar color(255, 255, 255);
	cv::Scalar color2(0, 0, 255);
	cv::Scalar color3(0, 0, 0);
	cv::Scalar color4(255, 0, 0);
	cv::RotatedRect bounding_rect;

	for (int i = 0; i < contours.size(); i++) // iterate through each contour.
	{
		double a = cv::contourArea(contours[i], false);  //  Find the area of contour
		bounding_rect = cv::minAreaRect(contours[i]);
		//cv::rectangle(frameBGR, bounding_rect, color2);
		// rotated rectangle
		cv::Point2f rect_points[4]; bounding_rect.points(rect_points);
		for (int j = 0; j < 4; j++)
			line(frameBGR, rect_points[j], rect_points[(j + 1) % 4], color2, 1, 8);

		drawContours(frameBGR, contours, i, color, -1, 8, hierarchy); // Draw the largest contour using previously stored index.
	}
	/*
	for (int i = 0; i < contours2.size(); i++) // iterate through each contour.
	{
		double a = cv::contourArea(contours2[i], false);  //  Find the area of contour
		bounding_rect = cv::minAreaRect(contours2[i]);
		//cv::rectangle(frameBGR, bounding_rect, color4);
		drawContours(frameBGR, contours2, i, color3, -1, 8, hierarchy2); // Draw the largest contour using previously stored index.
	}
	*/

		cv::imshow("i", innerThresholded);
		cv::imshow("o", outerThresholded);
#else
//	cv::Mat image;
//	frameBGR.copyTo(image);
//	image.copyTo(frameBGR, imgThresholded);
	for (int dir = 0; dir < 2; dir++) {
		int end = dir ? frameHSV.cols - 1 : 0;
		for (int y = 0; y < frameHSV.rows; y+=5) {
			int outer_start = -1;
			int outer_end = -1;
			int inner_start = -1;
			int inner_end = -1;

			for (int x = 0; x < frameHSV.cols; x++) {
				if (inner_end > 0 && inner_end - inner_start > 1) {
					for (int z = inner_end; z >= 0; z--) {
						outerThresholded.ptr<uchar>(y)[dir ? end -z : z] = (unsigned char)255;
						frameBGR.at<cv::Vec3b>(y, dir ? end -z : z) = cv::Vec3b(0, 255, 0);
						frameHSV.at<cv::Vec3b>(y, dir ? end - z : z) = cv::Vec3b(0, 255, 255);
					}
					for (int z = outer_end; z > outer_start; z--) {
						//outerThresholded.ptr<uchar>(y)[z] = (unsigned char)255;
						frameBGR.at<cv::Vec3b>(y, dir ? end - z : z) = cv::Vec3b(0, 255, 255);
					}
					break;
				}
				else if (inner_start > 0 && x - inner_start > 10 && innerThresholded.ptr<uchar>(y)[dir ? end -x : x] == 0) {
					inner_end = x;
				}
				else if (outer_end > 0 && x - outer_end < 150 && inner_start < 0 && innerThresholded.ptr<uchar>(y)[dir ? end -x : x] == 255) {
					inner_start = x;
				}
				else if (outer_end < 0 && outer_start > 0 && x - outer_start > 10 && outerThresholded.ptr<uchar>(y)[dir ? end -x : x] == 0) {
					outer_end = x;
				}
				else if (outer_start < 0 && outerThresholded.ptr<uchar>(y)[dir ? end -x : x] == 255) {
					outer_start = x;
				}
			}
		}
	}



//	cv::imshow("io", innerThresholded + outerThresholded);
//	cv::imshow("i", innerThresholded);
//	cv::imshow("o", outerThresholded);
#endif

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


