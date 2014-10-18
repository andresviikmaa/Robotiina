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
	
	WheelController(boost::asio::io_service &io);
	void Forward(int speed);
    void MoveTo(const CvPoint &);
    void Rotate(bool direction, int speed);
	void Drive(int velocity, double direction);
	void DriveRotate(int velocity, double direction, int rotate);
	bool DriveToBall(double distance, double horizontalDev, double horizontalAngle, int desiredDistance);
	void Stop();
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
