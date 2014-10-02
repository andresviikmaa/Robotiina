#pragma  once
#include "types.h"
#include "simpleserial.h"

class Wheel: public SimpleSerial
{
public:
	Wheel(boost::asio::io_service &io_service, std::string port = "port", unsigned int baud_rate = 115200) : SimpleSerial(io_service, port, baud_rate) {};
	void Run(int speed);
	void Stop();	
};