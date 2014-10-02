#pragma once

#include "wheel.h"
#include "types.h"

class WheelController {
private:
    CvPoint curLocation;
public:
    WheelController(){};
    void MoveTo(const CvPoint &){};
    void Rotate(double) {};
    ~WheelController(){};
};