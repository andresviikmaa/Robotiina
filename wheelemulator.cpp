
#include "wheelemulator.h"


WheelEmulator::WheelEmulator(boost::asio::io_service &io_service, std::string port, unsigned int baud_rate)
	: SimpleSerial(io_service, port, baud_rate)
{
	stop = false;
}

void WheelEmulator::Run() {
	while (!stop) {
		std::string message = readLine();
		writeString("stall:0\n");
	}

}
void WheelEmulator::Stop() {
	serial.cancel();
	stop = true;
}

WheelEmulator::~WheelEmulator()
{
}
