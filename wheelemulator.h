#pragma  once
#include "types.h"
#include "simpleserial.h"
#include <boost/atomic.hpp>
/*
	You can use Virtual Serial Port Driver from Eltima Software to bind two COM ports together
	and use this class instead of physical device
*/
class WheelEmulator : public SimpleSerial
{
public:
	boost::atomic<bool> stop;

	WheelEmulator(boost::asio::io_service &io_service, std::string port = "port", unsigned int baud_rate = 115200);
	void Run();
	void Stop();
	~WheelEmulator();
};

