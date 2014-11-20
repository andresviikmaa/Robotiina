#include "Audrino.h"
#include <thread>


void AudrinoBoard::Run(){
	std::string resp;
	while (!stop_thread){
		std::chrono::milliseconds dura(100);
		std::this_thread::sleep_for(dura);
		//std::cout << "start" << std::endl;
		
		writeString("snr0\n");
		std::this_thread::sleep_for(dura);
		sonars.x = atoi(readLineAsync(800).c_str());
		std::cout << "snr0: " << sonars.x << std::endl;
		std::this_thread::sleep_for(dura);

		writeString("snr2\n");
		std::this_thread::sleep_for(dura);
		sonars.y = atoi(readLineAsync(800).c_str());
		std::cout << "snr2: " << sonars.y << std::endl;
		std::this_thread::sleep_for(dura);

		sonars.z = -1;
		sonars.x = -1;
		sonars.y = -1;
	}
}


std::string Audrino::GetDebugInfo(){
	std::ostringstream oss;
	oss << "[Audrino] left: " << sonars.x;
	oss << ", right: " << sonars.z;
	oss << ", back " << sonars.y;
	
	return oss.str();
}


