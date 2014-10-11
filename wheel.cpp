#include "wheel.h"


void Wheel::Stop(){
	try {
		writeString("sd0\n");
		/* std::cout << readLine() << std::endl;*/

	}
	catch (boost::system::system_error& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return;
	}
}

void Wheel::Run(int given_speed){
	try {
		speed = given_speed;
		std::ostringstream oss;
		oss << "sd" << speed << "\n";
		writeString(oss.str());
		//std::cout << "WheelSpeed " << id << " " << speed << std::endl;
		return;

	}
	catch (boost::system::system_error& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return;
	}
}

int Wheel::Speed(){
	try {
		writeString("s\n");
		/*std::cout << readLine() << std::endl;*/
		return atoi(readLine().c_str());
	}
	catch (boost::system::system_error& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return 0;
	}
	
}

void Wheel::StallCheck(){
	
	while (!stop_thread){
		if (readLine() == "stall:1"){
			stall = true;
		}
	}
}