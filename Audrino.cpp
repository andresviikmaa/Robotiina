#include "Audrino.h"



void AudrinoBoard::Run(){
	std::string resp;
	while (!stop_thread){
		writeString("snr0\n");
		sonars.x = atoi(readLine().c_str());
		writeString("snr1\n");
		sonars.y = atoi(readLine().c_str());
		writeString("snr2\n");
		sonars.z = atoi(readLine().c_str());

	}
}


std::string Audrino::GetDebugInfo(){
	std::ostringstream oss;
	oss << "[Audrino] left: " << sonars.x;
	oss << ", right: " << sonars.y;
	oss << ", back " << sonars.z;

	return oss.str();
}


