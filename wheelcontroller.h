#pragma once

#include "wheel.h"
#include "types.h"
#include <boost/timer/timer.hpp>

class WheelController {
private:
    cv::Point3d lastSpeed; // x, y, heading
	cv::Point3d actualSpeed;
	BasicWheel * w_left;
	BasicWheel * w_right;
	BasicWheel * w_back;
	boost::posix_time::ptime stallTime = boost::posix_time::microsec_clock::local_time() + boost::posix_time::seconds(60);
protected:
	cv::Point3d CalculateWheelSpeeds(double velocity, double direction, double rotate);
public:
	WheelController();
	void InitWheels(boost::asio::io_service &io, bool useDummyPorts = false);
	void Forward(int speed);
    void MoveTo(const CvPoint &);
	cv::Point3d Rotate(bool direction, double speed);
	cv::Point3d Drive(double velocity, double direction);
	cv::Point3d DriveRotate(double velocity, double direction, double rotate);
	cv::Point3d Stop();
	cv::Point3d GetWheelSpeeds();
	void GetRobotSpeed(double &velocity, double &direction, double &rotate);
	void GetTargetSpeed(double &velocity, double &direction, double &rotate);
	bool IsStalled();
	bool HasError();
	~WheelController();
	void DestroyWheels();
};
