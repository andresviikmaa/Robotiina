#pragma  once
#include "types.h"
#include "simpleserial.h"

class Wheel
{
public:
	Wheel(std::string port, unsigned int baud_rate);
	void Run(int speed);
	void Stop();	
};