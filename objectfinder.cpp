#include "objectfinder.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <math.h>
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

bool ObjectFinder::Locate(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target, ObjectPosition &targetPos) {
	cv::Point2i point(-1, -1);
	cv::Scalar color(0, 0, 0);
	bool resetFilter = false;
	if (target == BALL){
		point = LocateBallOnScreen(HSVRanges, frameHSV, frameBGR, target);
		color = cv::Scalar(0, 225, 225);
	}
	else{
		point = LocateGateOnScreen(HSVRanges, frameHSV, frameBGR, target);
	}
	if (point.x < -1 && point.y < -1){//If ball is not valid then no predicting
		lastPosition = point;
		resetFilter = true;
		return false;
	}
	else if (point.x < 0 && point.y < 0){//If ball is suddenly lost then predict where it could be
		point = filter->getPrediction();
		lastPosition = point;
		if (point.x < 0 && point.y < 0){
			resetFilter = true;
			return false;
		}
	}
	else {//Ball is in frame
		if (resetFilter){
			resetFilter = false;
			filter->reset(point);
		}
		point = filter->doFiltering(point);
		lastPosition = point;
	}
	cv::circle(frameBGR, point, 8, color, -1);
	//std::cout << point << std::endl;
	targetPos = ConvertPixelToRealWorld(point, cv::Point2i(frameHSV.cols, frameHSV.rows));
	WriteInfoOnScreen(targetPos);
	return true;
}

cv::Point2i ObjectFinder::LocateGateOnScreen(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target) {
	int smallestGateArea = 1000;
	int growGateHeight = 1.2;
	cv::Point2d center(-1,-1);
	cv::Mat imgThresholded = HSVRanges[target]; // reference counted, I think
	cv::Mat dst(imgThresholded.rows, imgThresholded.cols, CV_8U, cv::Scalar::all(0));

	cv::Scalar color(0, 0, 0);
	cv::Scalar color2(255, 255, 255);

	//biggest area calculation
	std::vector<std::vector<cv::Point>> contours; // Vector for storing contour
	std::vector<cv::Vec4i> hierarchy;
	cv::Rect bounding_rect;

	findContours(imgThresholded, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // Find the contours in the image
	if (contours.size() == 0){
		return center;
	}

	double largest_area = 0;
	double area = 0;
	size_t largest_contour_index = 0;
	for (size_t i = 0; i < contours.size(); i++) // iterate through each contour.
	{
		area = cv::contourArea(contours[i], false);  //  Find the area of contour		
		if (area > largest_area){
			largest_area = area;
			largest_contour_index = i;                //Store the index of largest contour
		}
	}

	//validate gate area
	if (largest_area < smallestGateArea){
		return cv::Point2i(-1, -1);
	}
	
	//find center
	if (contours.size() > largest_contour_index){
		cv::Moments M = cv::moments(contours[largest_contour_index]);
		center = cv::Point2i(M.m10 / M.m00, M.m01 / M.m00);
	}
	else {
		assert(false);
	}

	//Cutting out gate from ball frame	
	bounding_rect = cv::boundingRect(contours[largest_contour_index]);
	bounding_rect.height = bounding_rect.height * growGateHeight;
	rectangle(HSVRanges[BALL], bounding_rect.tl(), bounding_rect.br(), color, -1, 8, 0);
	//for clear visual:
	rectangle(frameBGR, bounding_rect.tl(), bounding_rect.br(), color, -1, 8, 0);
	return center;
}

cv::Point2i ObjectFinder::LocateBallOnScreen(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR, OBJECT target) {
	int smallestBallArea = 10;
	cv::Point2d center(-1, -1);
	cv::Mat imgThresholded = HSVRanges[target]; // reference counted, I think
	cv::Mat dst(imgThresholded.rows, imgThresholded.cols, CV_8U, cv::Scalar::all(0));

	std::vector<std::vector<cv::Point>> contours; // Vector for storing contour
	std::vector<cv::Vec4i> hierarchy;

	cv::Scalar color(0, 0, 0);
	cv::Scalar color2(255, 255, 255);

	findContours(imgThresholded, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // Find the contours in the image

	if (contours.size() == 0){ //if no contours found
		return center;
	}
	//the geater the closest
	double ball_distance = 0;
	double closest_distance = 0;
	std::vector<std::pair<int, int> > ball_indexes;
	for (int i = 0; i < contours.size(); i++) // iterate through each contour.
	{
		int area = cv::contourArea(contours[i], false);
		if (area < smallestBallArea){ //validate ball's size
			ball_distance = 0;
		}
		else{
			cv::Moments M = cv::moments(contours[i]);
			ball_distance = M.m01 / M.m00;
			ball_indexes.push_back(std::make_pair(ball_distance, i));
		}
	}

	if (ball_indexes.empty()){
		return center;
	}

	int closest_ball_index = 0;
	cv::Point2d closestBall = cv::Point2d(-1, -1);
	std::sort(ball_indexes.begin(), ball_indexes.end());
	while (!ball_indexes.empty()){
		//If there is nothing to compare with
		closest_ball_index = ball_indexes.back().second;
		//Choosing new ball
 		if (lastPosition.x < 0 && lastPosition.y < 0){
			if (contours.size() > closest_ball_index){
				cv::Moments M = cv::moments(contours[closest_ball_index]);
				closestBall = cv::Point2d(M.m10 / M.m00, M.m01 / M.m00);
			}
			else {
				assert(false);
			}
			//If we found not valid ball
			if (cv::norm(closestBall - notValidPosition) < 100 && notValidPosition.x > 0 && notValidPosition.y > 0){
				return cv::Point2i(-2, -2);
			}
			else{
				notValidPosition = cv::Point2i(-1, -1); //reset variable
			}
		}
		//Comparing with prev result
		else{
			double smallestDistance = 9999;
			
			for (int i = 0; i < ball_indexes.size(); i++){
				cv::Moments M = cv::moments(contours[i]);
				center = cv::Point2d(M.m10 / M.m00, M.m01 / M.m00);
				double distance = cv::norm(center - lastPosition);
				if (smallestDistance > distance){
					smallestDistance = distance;
					closestBall = center;
				}
			}
			//distance between found ball and chosen ball
			if (smallestDistance > 60){
				return cv::Point2d(-1,-1);
			}
			
		}
		//VALIDATE BALL
		//For ball validation, drawed contour should cover balls shadow.
		int thickness = (int)ceil(cv::contourArea(contours[closest_ball_index], false) / 50);
		thickness = std::min(100, std::max(thickness, 12));
		drawContours(HSVRanges[INNER_BORDER], contours, closest_ball_index, color, thickness, 8, hierarchy);
		drawContours(HSVRanges[INNER_BORDER], contours, closest_ball_index, color, -5, 8, hierarchy);
		drawContours(HSVRanges[OUTER_BORDER], contours, closest_ball_index, color, thickness, 8, hierarchy);
		drawContours(HSVRanges[OUTER_BORDER], contours, closest_ball_index, color, -5, 8, hierarchy);

		bool valid = validateBall(HSVRanges, closestBall, frameHSV, frameBGR);
		if (!valid){
			notValidPosition = closestBall;
			cv::circle(frameBGR, closestBall, 5, cv::Scalar(100, 0, 225), -1); //not valid ball is purple
		}
		else{
			cv::circle(frameBGR, closestBall, 12, cv::Scalar(225, 225, 225), 3); //valid ball is white
			return	closestBall;
		}
		ball_indexes.pop_back();
	}
	//If there is not valid ball
	return cv::Point2i(-2, -2);

}

bool ObjectFinder::validateBall(ThresholdedImages &HSVRanges, cv::Point2d endPoint, cv::Mat &frameHSV, cv::Mat &frameBGR)
{

	cv::Mat innerThresholded = HSVRanges[INNER_BORDER];
	cv::Mat outerThresholded = HSVRanges[OUTER_BORDER];

	cv::Point startPoint;
	startPoint.x = frameHSV.cols / 2;
	startPoint.y = frameHSV.rows;

	cv::LineIterator iterator(frameHSV, startPoint, endPoint, 8);
	int behindLineCount = 0;
	int alterIterator = -10;
	for (int n = 0; n < 10; n++){
		cv::Point2d lastInner = {0,0};
		cv::Point2d firstInner = {0,0};
		cv::Point2d lastOuter = {0,0};
		cv::Point2d firstOuter = {0,0};

		std::string state = "inner";
		bool Hinrange = false;
		bool Sinrange = false;
		bool Vinrange = false;
		bool firstFound = false;
		iterator = cv::LineIterator{ frameHSV, cv::Point(startPoint.x + alterIterator, startPoint.y), cv::Point(endPoint.x + alterIterator, endPoint.y), 8 };
		for (int i = 0; i < iterator.count; i++, ++iterator)
		{
			if (state == "inner"){
				bool inRange = innerThresholded.ptr<uchar>(iterator.pos().y)[iterator.pos().x] == 255;
				if (inRange){
					firstInner = iterator.pos();
					state = "outer";
				}
			}
			else if (state == "outer"){
				bool inRange = outerThresholded.ptr<uchar>(iterator.pos().y)[iterator.pos().x] == 255;
				if (inRange && !firstFound){
					firstFound = true;
					firstOuter = iterator.pos();
				}
				else if (inRange){
					lastOuter = iterator.pos();
				}
				inRange = innerThresholded.ptr<uchar>(iterator.pos().y)[iterator.pos().x] == 255;
				if (inRange){
					lastInner = iterator.pos();
				}

			}
		}//lineiterator end
		alterIterator = alterIterator + 2;
		cv::circle(frameBGR, firstOuter, 5, cv::Scalar(0, 0, 0), -1);
		cv::circle(frameBGR, lastInner, 5, cv::Scalar(200, 200, 200), -1);

		double distLiFo = cv::norm(lastInner - firstOuter);

		if (!firstFound){
			continue;
		}
		if (distLiFo < 60){
			behindLineCount++;
		}
	}//ten times end
	
	return !(behindLineCount >= 2);
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

void ObjectFinder::IsolateField(ThresholdedImages &HSVRanges, cv::Mat &frameHSV, cv::Mat &frameBGR) {

	cv::Mat innerThresholded = HSVRanges[INNER_BORDER];
	cv::Mat outerThresholded = HSVRanges[OUTER_BORDER];

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

ObjectPosition ObjectFinder::ConvertPixelToRealWorld(const cv::Point2i &point, const cv::Point2i &frame_size)
{
	if (point.y >= 0 && point.x >= 0 && point.y < frame_size.y && point.x < frame_size.x){//If there is no object found

	}
		

	const cv::Point2d center (frame_size.x / 2.0, frame_size.y / 2.0);
	//Calculating distance
	double angle = (Vfov * (point.y - center.y) / center.y) + CamAngleDev;
	double distance = CamHeight / tan(angle * PI / 180);
	//Calculating horizontal deviation
	double hor_space = tan(Hfov)*distance;
	double HorizontalDev = (hor_space * (point.x - center.x) / center.x);
	double Hor_angle = atan(HorizontalDev / distance)* 180/PI;
	if (Hor_angle > 0){
		Hor_angle = 360 - Hor_angle;
	}
	Hor_angle = abs(Hor_angle);
	return{ distance, HorizontalDev, Hor_angle };
}


void ObjectFinder::WriteInfoOnScreen(const ObjectPosition &info){
	cv::Mat infoWindow(100, 250, CV_8UC3, cv::Scalar::all(0));
	std::ostringstream oss;
	oss << "Distance :" << info.distance;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	oss.str("");
	oss << "Horizontal Dev :" << info.horizontalDev;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 50), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	oss.str("");
	oss << "Horizontal angle :" << info.horizontalAngle;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 80), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	cv::namedWindow("Info Window");
	cv::imshow("Info Window", infoWindow);
	return;
}


