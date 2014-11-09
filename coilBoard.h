#pragma  once
#include "types.h"
#include "simpleserial.h"
#include <boost/atomic.hpp>
#include <boost/timer/timer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "ThreadedClass.h"

class CoilGun: public ThreadedClass
{
public:
	CoilGun(){};
	virtual ~CoilGun(){};
	virtual void ToggleTribbler(bool start){};
	virtual bool BallInTribbler(){ return rand() % 100 > 95 ; };
	virtual void Kick() {};
	void Run(){};
};

class CoilBoard : public CoilGun, SimpleSerial
{
private:
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime waitTime = time;
	boost::posix_time::time_duration waitDuration;
	boost::atomic<bool> ballInTribbler;
	volatile int ballInTribblerCount = 0;


public:
	CoilBoard(boost::asio::io_service &io_service, std::string port = "", unsigned int baud_rate = 115200) : SimpleSerial(io_service, port, baud_rate) {
		stop_thread = false;
		ballInTribbler = false;
	};
	virtual ~CoilBoard(){
		writeString("d\n");//discharge
	}
	void Kick();
	void ToggleTribbler(bool start);
	bool BallInTribbler();
	void Run();

};











