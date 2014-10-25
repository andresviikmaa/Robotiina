#include "coilBoard.h"

void CoilBoard::Kick(){
	writeString("k1000\n");
	return;
}

void CoilBoard::StartTribbler(){
	return;
}

void CoilBoard::StopTribbler(){
	return;
}

bool CoilBoard::BallInTribbler(){
	return false;
}

void CoilBoard::Ping(){
	while (!stop_thread){
		time = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration::tick_type waitDuration = (time - waitTime).total_milliseconds();
		if (waitDuration > 300){
			writeString("p\n");
		}
	}
}