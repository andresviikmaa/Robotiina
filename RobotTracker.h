#pragma once
#include "types.h"
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>

class WheelController;


class RobotTracker
{
private:
	WheelController *wheels;
	boost::atomic<bool> stop_thread;
	boost::thread_group threads;

public:
	RobotTracker(WheelController *wheels);
	void Run();
	void WriteInfoOnScreen(cv::Point3d acutual_speed, cv::Point3d target_speed, double dt);
	~RobotTracker();
};

