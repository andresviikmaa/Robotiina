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

void drawLine(cv::Mat & img, cv::Mat & img2, int dir, cv::Vec4f line, int thickness, CvScalar color)
{
	double theMult = std::max(img.cols, img.rows);
	// calculate start point
	cv::Point startPoint;
	startPoint.y = line[2] - theMult*line[0];// x0
	startPoint.x = line[3] - theMult*line[1];// y0
	// calculate end point
	cv::Point endPoint;
	endPoint.y = line[2] + theMult*line[0];//x[1]
	endPoint.x = line[3] + theMult*line[1];//y[1]

	// draw overlay of bottom lines on image
	cv::clipLine(cv::Size(img.cols, img.rows), startPoint, endPoint);
	std::vector <cv::Point2i> points;
	points.push_back(dir ? cv::Point2i(img.cols - 1, img.rows-1) : cv::Point2i(0, 0));
	points.push_back(cv::Point2i(img.cols - 1, 0));

	points.push_back(startPoint);
	points.push_back(endPoint);
	//cv::fillConvexPoly(img, points, cv::Scalar(0, 255, 0));
	cv::fillConvexPoly(img2, points, cv::Scalar(0, 255, 0));

	cv::line(img, startPoint, endPoint, color, thickness, 8, 0);

}

void ObjectFinder::IsolateField(const HSVColorRange &inner, const HSVColorRange &outer, const HSVColorRange &gate1, const HSVColorRange &gate2, cv::Mat &frameHSV, cv::Mat &frameBGR) {

	//	cv::imshow("Thresholded Image 3", imgHSV); //show the thresholded image
	cv::Mat innerThresholded;
	inRange(frameHSV, cv::Scalar(inner.hue.low, inner.sat.low, inner.val.low), cv::Scalar(inner.hue.high, inner.sat.high, inner.val.high), innerThresholded); //Threshold the image
	cv::Mat outerThresholded;
	inRange(frameHSV, cv::Scalar(outer.hue.low, outer.sat.low, outer.val.low), cv::Scalar(outer.hue.high, outer.sat.high, outer.val.high), outerThresholded); //Threshold the image

	cv::Mat gate1Thresholded;
	inRange(frameHSV, cv::Scalar(gate1.hue.low, gate1.sat.low, gate1.val.low), cv::Scalar(gate1.hue.high, gate1.sat.high, gate1.val.high), gate1Thresholded); //Threshold the image
	cv::Mat gate2Thresholded;
	inRange(frameHSV, cv::Scalar(gate2.hue.low, gate2.sat.low, gate2.val.low), cv::Scalar(gate2.hue.high, gate2.sat.high, gate2.val.high), gate2Thresholded); //Threshold the image

	std::vector<std::vector<cv::Point> > contours; // Vector for storing contour
	std::vector<cv::Vec4i> hierarchy;
	std::vector<std::vector<cv::Point> > contours2; // Vector for storing contour
	std::vector<cv::Vec4i> hierarchy2;

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

	outerThresholded = outerThresholded + gate1Thresholded + gate2Thresholded;
//	cv::Mat image;
//	frameBGR.copyTo(image);
//	image.copyTo(frameBGR, imgThresholded);
	//dir: left -> right, right -> left, top -> down
	for (int dir = 0; dir < 4; dir++) {
		std::vector<cv::Point2i> points;
		int end = dir ? frameHSV.cols - 1 : 0;
		//int last_y = -1;
		cv::Point2i last_point(-1, -1);
		cv::Point2i last_point2(-1, -1);
		for (int y = 0; y < frameHSV.rows; y += 5) {
			int outer_start = -1;
			int outer_end = -1;
			int inner_start = -1;
			int inner_end = -1;
			bool break2 = false;
			for (int x = 0; x < frameHSV.cols; x++) {
				if (inner_end > 0 && inner_end - inner_start > 1) {
					cv::Point2i new_point = cv::Point2i(y, dir ? end - inner_start : inner_start);
					// chekck the monotonicity
					if (last_point2.x != -1 &&  !(((last_point.x - new_point.x) >= 0) ^ ((last_point2.x - last_point.x) < 0))) { // direction change, not straight line
						break2 = true;
						break;
					}
					if (last_point.x != -1 && (new_point.x - last_point.x) > 30){ // to big gap, break
						break2 = true;
						break;
					}

					if (last_point.y != -1 && new_point.y - last_point.y < 20) {
						points.push_back(last_point);
						cv::circle(frameBGR, cv::Point2i(last_point.y, last_point.x), 4, dir ? cv::Scalar(255, 255, 100) : cv::Scalar(0, 20, 100), 10);
						for (int z = inner_end; z >= 0; z--) {
							outerThresholded.ptr<uchar>(y)[dir ? end - z : z] = (unsigned char)255;
							frameBGR.at<cv::Vec3b>(y, dir ? end - z : z) = cv::Vec3b(0, 255*dir, 0);
							frameHSV.at<cv::Vec3b>(y, dir ? end - z : z) = cv::Vec3b(0, 255, 255);
						}
						for (int z = outer_end; z > outer_start; z--) {
							//outerThresholded.ptr<uchar>(y)[z] = (unsigned char)255;
							frameBGR.at<cv::Vec3b>(y, dir ? end - z : z) = cv::Vec3b(0, 255 * dir, 255);
						}
					}
					last_point2 = last_point;
					last_point = new_point;
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
		if (points.size() > 2) {
			cv::Vec4f newLine;
			cv::fitLine(points, newLine, CV_DIST_L2, 0, 0.001, 0.001);
			drawLine(frameBGR, frameHSV, dir, newLine, 2+dir, cv::Scalar(255, 255*(dir+0.3), 0));
		}
		//break;
	}



	cv::imshow("io", innerThresholded + outerThresholded);
	cv::imshow("i", innerThresholded);
	cv::imshow("o", outerThresholded);
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


