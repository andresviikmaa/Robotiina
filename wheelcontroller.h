#pragma once

#include "wheel.h"
#include "types.h"

class WheelController {
private:
    cv::Point3d lastSpeed; // x, y, heading
	cv::Point3d actualSpeed;
	BasicWheel * w_left;
	BasicWheel * w_right;
	BasicWheel * w_back;
	

public:
	
	WheelController(boost::asio::io_service &io, bool useDummyPorts = false);
	void Forward(int speed);
    void MoveTo(const CvPoint &);
	cv::Point3d Rotate(bool direction, double speed);
	cv::Point3d Drive(double velocity, double direction);
	cv::Point3d DriveRotate(double velocity, double direction, double rotate);
	cv::Point3d Stop();

	bool IsStalled();
    ~WheelController(){
		if (w_left)
			delete w_left;
		if (w_right)
			delete w_right;
		if (w_back)
			delete w_back;
	};
};
