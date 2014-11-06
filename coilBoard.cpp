#include "coilBoard.h"
#include <chrono>
#include <thread>
#define TRIBBLER_QUEUE_SIZE 30
#define TRIBBLER_STATE_THRESHOLD 16

void CoilBoard::Kick(){
	writeString("k1000\n");
	return;
}

void CoilBoard::ToggleTribbler(bool start){
	if (start) {
		writeString("m0\n");
	}
	else{
		writeString("m0\n");
	}
	
	return;
}


bool CoilBoard::BallInTribbler(){
        boost::mutex::scoped_lock lock(historyMutex);
        bool ballInTribblerOld = ballInTribbler;
int falseCount = 0;
int maxFalseCount = 0;
        if (ballInTribbler) {
// if there is sequence of 4 falses then switch state
		for (int i=ballInTribblerHistory.size()-1; i>0; i--){
		   if(!ballInTribblerHistory[i]) falseCount++;
		   if(ballInTribblerHistory[i]) {
			maxFalseCount = std::max(falseCount, maxFalseCount);                
                      falseCount=0;
		   }
		}
		if(maxFalseCount > TRIBBLER_STATE_THRESHOLD) ballInTribbler = false;
	} else {
		ballInTribbler = ballInTribblerHistory[ballInTribblerHistory.size()-1];
        } 
	std::cout << "bit " << ballInTribbler << ", " << maxFalseCount << std::endl;
	return ballInTribbler;
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
	if (result >= 2){
		
		return true;
	}
	else{
		
		return false;
	}

}

void CoilBoard::Run(){
	writeString("c\n");
	while (!stop_thread){
		std::string line = readLineAsync(10);
		if(line == "true" || line == "false"){
		    boost::mutex::scoped_lock lock(historyMutex);

		   if(ballInTribblerHistory.size()>TRIBBLER_QUEUE_SIZE)
			ballInTribblerHistory.pop_front();
		   ballInTribblerHistory.push_back(line == "true");
 		}
		time = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration::tick_type waitDuration = (time - waitTime).total_milliseconds();
		if (waitDuration > 300){
			writeString("p\n");
			waitTime = time;
		} else {
			writeString("b\n");
		}
		
                std::chrono::milliseconds dura(10);
		std::this_thread::sleep_for(dura);
	}
	writeString("d\n");
	writeString("m0\n");
}




















