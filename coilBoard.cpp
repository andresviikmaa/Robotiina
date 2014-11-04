#include "coilBoard.h"

void CoilBoard::Kick(){
	writeString("k1000\n");
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
	int result = 0;
	try{
		for (int i = 0; i < 10; i++){
			writeString("b\n");
			std::string line = readLine();
			if (line == "true"){
				result ++;
			}
		}
		
	}
	catch (boost::system::system_error& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return 0;
	}
	if (result >= 8){
		return true;
	}
	else{
		return false;
	}

}

void CoilBoard::Run(){
	while (!stop_thread){
		time = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration::tick_type waitDuration = (time - waitTime).total_milliseconds();
		if (waitDuration > 300){
			writeString("p\n");
			waitTime = time;
		}
	}
}