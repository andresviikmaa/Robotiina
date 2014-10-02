#include "wheel.h"


void Wheel::Stop(){
	try {
		writeString("sd0\n");
		std::cout << readLine() << std::endl;

	}
	catch (boost::system::system_error& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return;
	}
}

void Wheel::Run(int speed){
	try {
		std::ostringstream oss;
		oss << "sd" << speed << "\n";
		writeString(oss.str());
		std::cout << readLine() << std::endl;

	}
	catch (boost::system::system_error& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return;
	}
}