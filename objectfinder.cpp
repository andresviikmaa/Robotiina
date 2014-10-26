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

cv::Point3d ObjectFinder::Locate(const HSVColorRange &r, cv::Mat &frameHSV, cv::Mat &frameBGR, bool gate,const HSVColorRange &inner, const HSVColorRange &outer) {
	cv::Point2f point = LocateOnScreen(r, frameHSV, frameBGR, gate);
	bool valid = true;
	cv::Point3d info = cv::Point3d(-1,-1,-1);
	if (!gate){
		valid = validateBall(inner, outer, point, frameHSV, frameBGR);
	}
	if (valid){
		info = ConvertPixelToRealWorld(point, cv::Point2i(frameHSV.cols, frameHSV.rows));
	}
	
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

bool ObjectFinder::validateBall(const HSVColorRange &inner, const HSVColorRange &outer,cv::Point2f endPoint, cv::Mat &frameHSV, cv::Mat &frameBGR)
{
	cv::Point startPoint;
	startPoint.x = frameHSV.cols / 2;
	startPoint.y = frameHSV.rows;
	cv::LineIterator iterator(frameHSV, startPoint, endPoint, 1);

	for (int i = 0; i < iterator.count; i++, ++iterator)
	{
		(*iterator)[0] = 200;
	}

	return true;
}

void drawLine(cv::Mat & img, cv::Mat & img2, int dir, cv::Vec4f line, int thickness, CvScalar color)
{
	//std::cout << "drawLine: " << dir << std::endl;
	double theMult = std::max(img.cols, img.rows);
	// calculate start point
	cv::Point startPoint;
	startPoint.x = line[2] - theMult*line[0];// x0
	startPoint.y = line[3] - theMult*line[1];// y0
	// calculate end point
	cv::Point endPoint;
	endPoint.x = line[2] + theMult*line[0];//x[1]
	endPoint.y = line[3] + theMult*line[1];//y[1]

	// draw overlay of bottom lines on image
	cv::clipLine(cv::Size(img.cols, img.rows), startPoint, endPoint);
	std::vector <cv::Point2i> points;

	if (dir == 0) {
		if (endPoint.x == img.cols-1) return; // invalid, cuts out bottom half

		points.push_back(cv::Point2i(img.cols - 1, 0));
		points.push_back(cv::Point2i(0, 0));
		points.push_back(cv::Point2i(0, img.rows - 1));
		points.push_back(startPoint);
		points.push_back(endPoint);
		//cv::fillConvexPoly(img, points, cv::Scalar(0, 255, 0));
		cv::fillConvexPoly(img2, points, cv::Scalar(0, 255, 0));
	}
	else if (dir == 1) {
		points.push_back(cv::Point2i(0, 0));
		points.push_back(cv::Point2i(img.cols - 1, 0));
		points.push_back(cv::Point2i(img.cols - 1, img.rows - 1));
		points.push_back(endPoint);
		points.push_back(startPoint);
		//cv::fillConvexPoly(img, points, cv::Scalar(0, 255, 0));
		cv::fillConvexPoly(img2, points, cv::Scalar(0, 255, 0));

	}
	cv::line(img, startPoint, endPoint, color, thickness, 8, 0);

}

void ObjectFinder::IsolateField(const HSVColorRange &inner, const HSVColorRange &outer, const HSVColorRange &gate1, const HSVColorRange &gate2, cv::Mat &frameHSV, cv::Mat &frameBGR) {

	//	cv::imshow("Thresholded Image 3", imgHSV); //show the thresholded image
	cv::Mat innerThresholded;
	inRange(frameHSV, cv::Scalar(inner.hue.low, inner.sat.low, inner.val.low), cv::Scalar(inner.hue.high, inner.sat.high, inner.val.high), innerThresholded); //Threshold the image
	cv::Mat outerThresholded;
	inRange(frameHSV, cv::Scalar(outer.hue.low, outer.sat.low, outer.val.low), cv::Scalar(outer.hue.high, outer.sat.high, outer.val.high), outerThresholded); //Threshold the image
	/*
	cv::Mat gate1Thresholded;
	inRange(frameHSV, cv::Scalar(gate1.hue.low, gate1.sat.low, gate1.val.low), cv::Scalar(gate1.hue.high, gate1.sat.high, gate1.val.high), gate1Thresholded); //Threshold the image
	cv::Mat gate2Thresholded;
	inRange(frameHSV, cv::Scalar(gate2.hue.low, gate2.sat.low, gate2.val.low), cv::Scalar(gate2.hue.high, gate2.sat.high, gate2.val.high), gate2Thresholded); //Threshold the image
	*/


	/* 
	cv::Point2d orgin(frameBGR.cols / 2, frameBGR.rows *0.9);
	cv::circle(frameBGR, orgin, 40, cv::Scalar(40, 20, 100), 10);
	for (float a = -PI; a < 0 ; a += PI/10) {
		bool was_border_start = false;
		for (float d = 0; d < frameBGR.cols ; d += 10) {
			float x = d * std::cos(a);
			float y = d * std::sin(a);
			if (abs(x) > orgin.x) break;
			if (y < -orgin.y) break;

			cv::Point2i point = orgin + cv::Point2d(x, y);
			bool border = abs(x) > orgin.x * 0.9 || y < -orgin.y*0.9;
			if (!border) {
				bool is_border_start = innerThresholded.ptr<uchar>(point.y)[point.x] == 255;
				bool is_border_end = outerThresholded.ptr<uchar>(point.y)[point.x] == 255;

				border = !is_border_start && is_border_end && was_border_start;
				was_border_start = is_border_start;
			}
			cv::circle(frameBGR, point, 4, border? cv::Scalar(d, 255, 100 * a)  : cv::Scalar(d, 20, 100 * a), 10);
			if (border) break;
		}
		//break;
	}
	*/

//	cv::Mat gateThresholded = gate1Thresholded + gate2Thresholded;

	//dir: left -> right, right -> left, top -> down
	for (int dir = 0; dir < 2; dir++) {
		std::vector<cv::Point2i> points;
		int end = dir ? frameHSV.cols - 1 : 0;
		//int last_y = -1;
		cv::Point2i last_canditate(-1, -1);
		int dx = 0;
		cv::Point2i last_selected(-1, -1);
		for (int y = 0; y < frameHSV.rows; y += 5) {
			int outer_start = -1;
			int outer_end = -1;
			int inner_start = -1;
			int inner_end = -1;
			bool break2 = false;
			for (int x = 0; x < frameHSV.cols; x += 3) { // TODO: start near the last found piont
				//cv::circle(frameBGR, cv::Point2i(dir ? end - x : x, y), 1, dir ? cv::Scalar(255, 0, 0) : cv::Scalar(120, 0, 0), 10);
				if (inner_end > 0 && inner_end - inner_start > 1) {
					// we have detected new point on the border
					if (last_canditate.x != -1) {
						// check the distance form last candidate
						if (abs(y - last_canditate.y) < 40 && abs(x - last_canditate.x) < 40) { // close enough
							cv::Point2i new_point = cv::Point2i(dir ? end - last_selected.x : last_selected.x, last_selected.y);
							cv::circle(frameBGR, new_point, 4, dir ? cv::Scalar(255, 255, 100) : cv::Scalar(0, 20, 100), 10);
							points.push_back(new_point);
							dx = last_selected.x - outer_start;
							if (points.size() > 7) {
								break2 = true;
								break;
							}
						}
						else {
							// to big gap, either restart or stop
							if (points.size() < 3) {
								points.clear();
								cv::Point2i last_canditate(-1, -1);
								cv::Point2i last_selected(-1, -1);
								//break;
							}
							else {
								break2 = true;
								break;
							}
						}

					}
					{
						last_canditate = cv::Point2i(x, y);
						last_selected = cv::Point2i(inner_start, y);
						//cv::Point2i new_point = cv::Point2i(dir ? end - last_selected.x : last_selected.x, last_selected.y);
						//cv::circle(frameBGR, new_point, 1, !dir ? cv::Scalar(255, 255, 100) : cv::Scalar(0, 20, 100), 10);
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
			if (break2) break;
		}
		if (points.size() > 3) {
			cv::Vec4f newLine;
			cv::fitLine(points, newLine, CV_DIST_L2, 0, 0.1, 0.1);
			drawLine(frameBGR, frameHSV, dir, newLine, 2+dir, cv::Scalar(0, 255*(dir+0.3), 0));
		}
		//break;
	}


	/*
	cv::imshow("io", innerThresholded + outerThresholded);
	cv::imshow("i", innerThresholded);
	cv::imshow("o", outerThresholded);
	*/


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


