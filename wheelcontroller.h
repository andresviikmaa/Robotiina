#pragma once

#include "wheel.h"
#include "types.h"

class WheelController {
private:
    CvPoint curLocation;
	DummyWheel * w_left;
	DummyWheel * w_right;
	DummyWheel * w_back;
	

public:
	
	WheelController(boost::asio::io_service &io, bool useDummyPorts = false);
	void Forward(int speed);
    void MoveTo(const CvPoint &);
	cv::Point2f Rotate(bool direction, int speed);
	cv::Point2f Drive(int velocity, double direction);
	cv::Point2f DriveRotate(int velocity, double direction, int rotate);
	cv::Point2f DriveToBall(double distance, double horizontalDev, double horizontalAngle, int desiredDistance);
	cv::Point2f Stop();

	bool CheckStall();
    ~WheelController(){
		if (w_left)
			delete w_left;
		if (w_right)
			delete w_right;
		if (w_back)
			delete w_back;
	};
};
