#include "wheel.h"


Wheel::Wheel(std::string port, unsigned int baud_rate){
	try {

		SimpleSerial serial("/dev/ttyUSB0", 115200);

		serial.writeString("100\n");

		std::cout << serial.readLine() << std::endl;

	}
	catch (boost::system::system_error& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return;
	}
}

void Wheel::Stop(){
	//Sending stop message to serial port
}

void Wheel::Run(int speed){
	//Sending run speed to serial port
}