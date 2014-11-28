#include "Arduino.h"
#include <thread>


void ArduinoBoard::Run(){
	std::string resp;
	while (!stop_thread){
		std::chrono::milliseconds dura(120);
		
		writeString("snr1\n");
		std::this_thread::sleep_for(dura);
		resp = readLineAsync(800).c_str();
		sonars.x = atoi(resp.c_str());
		//std::cout << "snr0: " << sonars.x << std::endl;

		std::this_thread::sleep_for(dura);

		writeString("snr2\n");
		std::this_thread::sleep_for(dura);
		sonars.y = atoi(readLineAsync(800).c_str());
		//std::cout << "snr2: " << sonars.y << std::endl;
		sonars.z = -1;
		
		std::this_thread::sleep_for(dura);

		writeString("strt\n");
		std::this_thread::sleep_for(dura);
		strt = atoi(readLineAsync(800).c_str());
		
		std::this_thread::sleep_for(dura);
		
		writeString("gte\n");
		std::this_thread::sleep_for(dura);
		gte = atoi(readLineAsync(800).c_str());
		
		std::this_thread::sleep_for(dura);
		
	}
}


std::string Arduino::GetDebugInfo(){
	std::ostringstream oss;
	oss << "[Arduino] left: " << sonars.x;
	oss << ", right: " << sonars.z;
	oss << ", back " << sonars.y;
	oss << ", start " << strt;
	oss << ", gate " << gte;
	
	return oss.str();
}


