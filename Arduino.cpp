#include "Arduino.h"
#include <thread>
#include <boost/algorithm/string.hpp>


void ArduinoBoard::Run(){
	//return Run2();
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

void ArduinoBoard::Run2(){
	std::string resp;
	std::chrono::milliseconds dura(120);
	std::vector<std::string> items;
	std::vector<std::string> item;
	while (!stop_thread){
		
		writeString("info\n");
		std::this_thread::sleep_for(dura);
		resp = readLineAsync(800).c_str();
		std::cout << "Arduino, resp='"<< resp <<"'"<<std::endl;
 		//"<start:"+strt+">"+"<gate:"+gate+">"+"<s1:"+distance0+">"+"<s2:"+distance1+">"+"<s3:"+distance2+">"
		resp = ">" + resp;
		boost::split(items, resp, boost::is_any_of(">"));
		for(auto i : items) {
			boost::split(item, i, boost::is_any_of(":"));
			if (item.size()==2) {
				if(item[0] == "<start") strt = atoi(item[1].c_str());
				if(item[0] == "<gate") gte = atoi(item[1].c_str());
				if(item[0] == "<s1") sonars.x = atoi(item[1].c_str());
				if(item[0] == "<s2") sonars.y = atoi(item[1].c_str());
				if(item[0] == "<s3") sonars.z = atoi(item[1].c_str());
			} else {
				std::cout << "Arduino, invalid responce item: '" << i << "', " << ", resp: '" << resp << "'" << std::endl; 
			}
		}

		sonars.x = atoi(resp.c_str());
		sonars.y = atoi(readLineAsync(800).c_str());
		sonars.z = -1;
		strt = atoi(readLineAsync(800).c_str());
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


