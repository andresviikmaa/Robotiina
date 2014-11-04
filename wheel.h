#pragma  once
#include "types.h"
#include "simpleserial.h"
#include <boost/atomic.hpp>
#include <boost/timer/timer.hpp>
#include "ThreadedClass.h"

class BasicWheel : public ThreadedClass
{
public:
	BasicWheel();
	virtual ~BasicWheel();
	void SetSpeed(int given_speed);
	int GetSpeed() {
		return actual_speed;
	};
	double GetSpeedInRPM() {
		return actual_speed * 62.5 / (18.75 * 45) * 60;
	};
	int GetDistanceTraveled(bool reset = true);
	bool IsStalled() {
		return stall;
	}
	void Stop();

protected:
	boost::atomic<bool> stall;
	//boost::mutex mutex;
	boost::atomic<int> target_speed;
	boost::atomic<int> actual_speed;
	int last_speed = 0;
	boost::atomic<bool> update_speed;
	int id = 0;


	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime lastStep = time;
	boost::posix_time::ptime lastUpdate = time;
	boost::posix_time::ptime stallTime = time;
	boost::posix_time::time_duration stallDuration;
	long distance_traveled = 0;
	void CheckStall();
	virtual void UpdateSpeed() = 0;
	void Run();

};

class SoftwareWheel : public BasicWheel
{
protected:
	int max_acceleration = 500;
	double stop_time = 1600;
	boost::posix_time::ptime stallStart;
	void UpdateSpeed();
};


class SerialWheel : public BasicWheel, SimpleSerial
{
protected:
	void UpdateSpeed();
public:
	SerialWheel(boost::asio::io_service &io_service, std::string port = "port", unsigned int baud_rate = 115200) : SimpleSerial(io_service, port, baud_rate) {
	};


};
