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
	speed = given_speed;
	update_speed = true;
        return;
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

std::string Wheel::Speed(){
	try {
		if(update_speed){
			std::ostringstream oss;
			oss << "sd" << speed << "\n";
			writeString(oss.str());
			update_speed = false;
		}
		writeString("s\n");
		return readLine();
	}
	catch (boost::system::system_error& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return 0;
	}
	
}

void Wheel::StallCheck(){
	
	while (!stop_thread){
		time = boost::posix_time::microsec_clock::local_time();
		std::string line = Speed();
		//std::cout<< "line: " << line << std::endl ;
 		actual_speed = atoi(line.substr(3).c_str());
		int diff = abs(actual_speed - speed);
		boost::posix_time::time_duration::tick_type stallDuration = (time - stallTime).total_milliseconds();
		if (diff > 10){
			std::cout<< "diff: " << diff << std::endl;

			if (stallDuration > 400){stall=true;}
		}
		else{
			stallTime = time;
			stall = false;
		}
		
		/*if (line == "<stall:1>" || line == "<stall:2>"){
			stall = true;
		}
		else{
			stall = false;
		}*/
	}
}
