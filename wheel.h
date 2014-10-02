#pragma  once
#include "types.h"
#include "simpleserial.h"

class Wheel: public SimpleSerial
{
public:
	Wheel(std::string port = "port" , unsigned int baud_rate = 115200) : SimpleSerial(port, baud_rate) {};
	void Run(int speed);
	void Stop();	
};