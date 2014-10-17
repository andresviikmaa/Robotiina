#pragma  once
#include "types.h"
#include "simpleserial.h"
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>

//TODO: rename
class DummyWheel
{
public:
	DummyWheel(){};
	virtual ~DummyWheel(){};
	virtual void Run(int given_speed){};
	virtual void Stop(){};
	virtual int Speed(){ return 0; };
	bool stall;

};

class Wheel : public DummyWheel, SimpleSerial
{
private:
	int speed = 0;
	int id = 0;
	void StallCheck();
	boost::atomic<bool> stop_thread;
	boost::thread_group threads;
public:
	Wheel(boost::asio::io_service &io_service, std::string port = "port", unsigned int baud_rate = 115200) : SimpleSerial(io_service, port, baud_rate) {
               stop_thread = false;
		threads.create_thread(boost::bind(&Wheel::StallCheck, this));
	};
	virtual ~Wheel(){
		stop_thread = true;
		threads.join_all();
	}
	void Run(int given_speed);
	void Stop();
	int Speed();
	
};
