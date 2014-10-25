#pragma  once
#include "types.h"
#include "simpleserial.h"
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/timer/timer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

class CoilBoard : SimpleSerial
{
private:
	void Ping();
	boost::atomic<bool> stop_thread;
	boost::thread_group threads;
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime waitTime = time;
	boost::posix_time::time_duration waitDuration;
public:
	CoilBoard(boost::asio::io_service &io_service, std::string port, unsigned int baud_rate = 115200) : SimpleSerial(io_service, port, baud_rate) {
		using boost::property_tree::ptree;
		ptree pt;
		read_ini("conf/ports.ini", pt);
		port = pt.get<std::string>(std::to_string(ID_COILGUN));
		stop_thread = false;
		threads.create_thread(boost::bind(&CoilBoard::Ping, this));
	};
	virtual ~CoilBoard(){
		stop_thread = true;
		threads.join_all;
		writeString("d\n");//discharge
	}
	void Kick();
	void ToggleTribbler();
	bool BallInTribbler();

};