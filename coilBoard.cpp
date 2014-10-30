#include "coilBoard.h"

void CoilBoard::Kick(){
	writeString("k1000\n");
	return;
}

void CoilBoard::ToggleTribbler(bool start){
	writeString("m\n");
	return;
}


bool CoilBoard::BallInTribbler(){
	try{
		writeString("b\n");
		std::string line = readLine();
		if (line == "true"){
			return true;
		}
		else{
			return false;
		}
	}
	catch (boost::system::system_error& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return 0;
	}

}

void CoilBoard::Ping(){
	while (!stop_thread){
		time = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration::tick_type waitDuration = (time - waitTime).total_milliseconds();
		if (waitDuration > 300){
			writeString("p\n");
			waitTime = time;
		}
	}
}