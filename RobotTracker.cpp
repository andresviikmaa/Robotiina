#include <thread>
#include "RobotTracker.h"
#include "wheelcontroller.h"

RobotTracker::RobotTracker(WheelController *wheels) :wheels(wheels)
{
	threads.create_thread(boost::bind(&RobotTracker::Run, this));

}

void RobotTracker::Run()
{
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime lastStep = time;
	double velocity = 0, direction = 0, rotate = 0;
	double velocity2 = 0, direction2 = 0, rotate2 = 0;

	while (!stop_thread) {
		time = boost::posix_time::microsec_clock::local_time();
		double dt = (time - lastStep).total_milliseconds();
		wheels->GetRobotSpeed(velocity, direction, rotate);
		wheels->GetTargetSpeed(velocity2, direction2, rotate2);
		WriteInfoOnScreen(cv::Point3d(velocity, direction, rotate), cv::Point3d(velocity2, direction2, rotate2), dt);
		lastStep = time;
		// speed update interval is 62.5Hz
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // do not poll serial to fast

	}
}


RobotTracker::~RobotTracker()
{
	stop_thread = true;
	threads.join_all();
}

void RobotTracker::WriteInfoOnScreen(cv::Point3d acutual_speed, cv::Point3d target_speed, double dt){
	cv::Mat infoWindow(150, 450, CV_8UC3, cv::Scalar::all(0));
	std::ostringstream oss;
	oss << "velocity :" << target_speed.x;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	oss.str("");
	oss << "direction :" << target_speed.y;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 50), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	oss.str("");
	oss << "rotate :" << target_speed.z;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 80), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	
	oss.str("");
	oss << "" << acutual_speed.x;
	cv::putText(infoWindow, oss.str(), cv::Point(220, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	oss.str("");
	oss << "" << acutual_speed.y;
	cv::putText(infoWindow, oss.str(), cv::Point(220, 50), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	oss.str("");
	oss << "" << acutual_speed.z;
	cv::putText(infoWindow, oss.str(), cv::Point(220, 80), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));


	oss.str("");
	oss << "dt :" << dt;
	cv::putText(infoWindow, oss.str(), cv::Point(20,110), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));


	cv::imshow("RobotTracker", infoWindow);
	cv::waitKey(1);
	return;
}


