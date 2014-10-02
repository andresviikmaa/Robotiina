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
	WheelController(Wheel left, Wheel right, Wheel back){
		w_left = left;
		w_back = back;
		w_right = right;
	};
	void Forward(int speed);
    void MoveTo(const CvPoint &);
    void Rotate(double degree);
    ~WheelController(){};
};