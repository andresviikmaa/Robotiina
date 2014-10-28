#pragma once

#include "wheel.h"
#include "types.h"
#include "coilBoard.h"

class WheelController {
private:
    cv::Point3f lastSpeed; // x, y, heading
	DummyWheel * w_left;
	DummyWheel * w_right;
	DummyWheel * w_back;
	

public:
	
	WheelController(boost::asio::io_service &io, bool useDummyPorts = false);
	void Forward(int speed);
    void MoveTo(const CvPoint &);
	cv::Point3f Rotate(bool direction, int speed);
	cv::Point3f Drive(int velocity, double direction);
	cv::Point3f DriveRotate(int velocity, double direction, int rotate);
	cv::Point3f DriveToBall(double distance, double horizontalDev, double horizontalAngle, int desiredDistance, DummyCoilBoard *coilBoard);
	cv::Point3f Stop();

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
