#pragma once

#include "wheel.h"
#include "types.h"

class WheelController {
private:
    CvPoint curLocation;
	Wheel * w_left;
	Wheel * w_right;
	Wheel * w_back;
	

public:
	
	WheelController(boost::asio::io_service &io);
	void Forward(int speed);
    void MoveTo(const CvPoint &);
    void Rotate(bool direction, int speed);
	void Drive(int velocity, double direction);
	void DriveRotate(int velocity, double direction, int rotate);
	void Stop();
	bool CheckStall();
    ~WheelController(){

		delete w_left;
		delete w_right;
		delete w_back;

	};
};