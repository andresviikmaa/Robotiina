#pragma  once
#include "types.h"
#include "simpleserial.h"
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/timer/timer.hpp>

//TODO: rename
class DummyWheel
{
public:
	DummyWheel(){};
	virtual ~DummyWheel(){};
	virtual void Run(int given_speed){};
	virtual void Stop(){};
	virtual std::string Speed(){ return 0; };
	bool stall;

};

class Wheel : public DummyWheel, SimpleSerial
{
private:
	boost::atomic<int> speed;
        boost::atomic<bool> update_speed;
        int actual_speed;
	int id = 0;
	void StallCheck();
	boost::atomic<bool> stop_thread;
	boost::thread_group threads;
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime stallTime = time;
	boost::posix_time::time_duration stallDuration;
public:
	Wheel(boost::asio::io_service &io_service, std::string port = "port", unsigned int baud_rate = 115200) : SimpleSerial(io_service, port, baud_rate) {
               stop_thread = false;
               speed = actual_speed = 0;
               update_speed = false;
		threads.create_thread(boost::bind(&Wheel::StallCheck, this));
	};
	virtual ~Wheel(){
		stop_thread = true;
		threads.join_all();
	}
	void Run(int given_speed);
	void Stop();
	std::string Speed();
	
};
