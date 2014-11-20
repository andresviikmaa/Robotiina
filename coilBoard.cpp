#include "coilBoard.h"
#include <chrono>
#include <thread>
#define TRIBBLER_QUEUE_SIZE 30
#define TRIBBLER_STATE_THRESHOLD 16

void CoilBoard::Kick(){
	writeString("k700\n");
	forcedNotInTribbler = true;
	afterKickTime = time; //reset timer
	return;
}

void CoilBoard::ToggleTribbler(bool start){
	if (start) {
		writeString("m1\n");
	}
	else{
		writeString("m0\n");
	}
	
	return;
}


bool CoilBoard::BallInTribbler(){

	return ballInTribblerCount > 0;
}

void CoilBoard::Run(){
	writeString("c\n");
	while (!stop_thread){
		std::string line = readLineAsync(10);
		if(line == "true" || line == "false" && !forcedNotInTribbler){
			//std::cout << "ballInTribblerCount " << ballInTribblerCount << " " << line << std::endl;
			int newcount = ballInTribblerCount + ((line == "true") ? 1 : -1);
			//std::cout << "ballInTribblerCount " << ballInTribblerCount << " " << newcount << " " << line << std::endl;
			ballInTribblerCount = std::min(2, std::max(-2, newcount));
 		}
		//Pinging
		time = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration::tick_type waitDuration = (time - waitTime).total_milliseconds();
		if (waitDuration > 300){
			writeString("p\n");
			waitTime = time;
		} else {
			writeString("b\n");
		}
		//Forcing ballintribler false after kick
		boost::posix_time::time_duration::tick_type afterKickDuration = (time - afterKickTime).total_milliseconds();
		if (afterKickDuration > 1000 && forcedNotInTribbler){
			forcedNotInTribbler = false;
		}
		else if (forcedNotInTribbler){
			ballInTribblerCount = -1;
		}
		std::chrono::milliseconds dura(10);
		std::this_thread::sleep_for(dura);
	}
	writeString("d\n");
	writeString("m0\n");
}




















