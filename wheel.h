#pragma  once
#include "types.h"
#include "simpleserial.h"
#include <boost/thread/thread.hpp>

class Wheel: public SimpleSerial
{
private:
	int speed = 0;
	int id = 0;
	void StallCheck();
	bool stop_thread = false;
	boost::thread_group threads;
public:
	Wheel(boost::asio::io_service &io_service, std::string port = "port", unsigned int baud_rate = 115200) : SimpleSerial(io_service, port, baud_rate) {
		threads.create_thread(boost::bind(&Wheel::StallCheck, this));
	};
	~Wheel(){
		stop_thread = true;
		threads.join_all();
	}
	std::string Run(int given_speed);
	void Stop();
	int Speed();
	bool stall;

	
	
};