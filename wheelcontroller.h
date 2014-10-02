#pragma once

#include "wheel.h"
#include "types.h"
class WheelController {
private:
    CvPoint curLocation;
	Wheel w_left;
	Wheel w_right;
	Wheel w_back;
public:
	WheelController():
		w_left(io_service, "port1", 115200),
		w_right(io_service, "port2", 115200),
		w_back(io_service, "port3", 115200)
	{
	};
	void Forward(int speed);
    void MoveTo(const CvPoint &);
    void Rotate(double degree);
    ~WheelController(){};
	boost::asio::io_service io_service;
};