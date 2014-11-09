
#include "AutoPilot.h"
#include "coilBoard.h"
#include "wheelcontroller.h"
#include <thread>

std::pair<DriveMode, std::string> DriveModes[] = {
	std::pair<DriveMode, std::string>(IDLE, "IDLE"),
	std::pair<DriveMode, std::string>(LOCATE_BALL, "LOCATE_BALL"),
	std::pair<DriveMode, std::string>(DRIVE_TO_BALL, "DRIVE_TO_BALL"),
	std::pair<DriveMode, std::string>(LOCATE_GATE, "LOCATE_GATE"),
	std::pair<DriveMode, std::string>(CATCH_BALL, "CATCH_BALL"),
	std::pair<DriveMode, std::string>(RECOVER_CRASH, "RECOVER_CRASH"),

	//	std::pair<STATE, std::string>(STATE_END_OF_GAME, "End of Game") // this is intentionally left out

};

std::map<DriveMode, std::string> DRIVEMODE_LABELS(DriveModes, DriveModes + sizeof(DriveModes) / sizeof(DriveModes[0]));

AutoPilot::AutoPilot(WheelController *wheels, CoilGun *coilgun) :wheels(wheels), coilgun(coilgun)
{
	stop_thread = false;
	threads.create_thread(boost::bind(&AutoPilot::Run, this));
}

void AutoPilot::UpdateState(ObjectPosition *ballLocation, ObjectPosition *gateLocation)
{
	boost::mutex::scoped_lock lock(mutex);
	ballInSight = ballLocation != NULL;
	gateInSight = gateLocation != NULL;
	if (ballInSight) lastBallLocation = *ballLocation;
	if (gateInSight) lastGateLocation = *gateLocation;
	ballInTribbler =  coilgun->BallInTribbler();
	lastUpdate = boost::posix_time::microsec_clock::local_time();
	if (driveMode == IDLE) driveMode = LOCATE_BALL;
}

/*
No ball in sight
*/
DriveMode AutoPilot::LocateBall() {
	if (ballInTribbler){
		return LOCATE_GATE;
	}

	if (ballInSight) return DRIVE_TO_BALL;
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime rotateStart = time;
	boost::posix_time::ptime rotateTime = time;
	while (!ballInSight) {
		if (ballInTribbler){
			return LOCATE_GATE;
		}
		if (stop_thread) return EXIT;
		if ((boost::posix_time::microsec_clock::local_time() - lastUpdate).total_milliseconds() > 1000) return IDLE;

		if (wheels->IsStalled()) return RECOVER_CRASH;

		time = boost::posix_time::microsec_clock::local_time();
		if ((time - rotateStart).total_milliseconds() > 10000) { // give up after 10 sec or perhaps go to different search mode
			return IDLE;
		}
		boost::posix_time::time_duration::tick_type rotateDuration = (time - rotateTime).total_milliseconds();
		if (false && rotateDuration >= 500){
			wheels->Stop();
			if (rotateDuration >= 600){
				rotateTime = time; //reset
			}
		}
		else{
			wheels->Rotate(1, 10);
		}


		std::chrono::milliseconds dura(10);
		std::this_thread::sleep_for(dura);
	}
	return DRIVE_TO_BALL;
}

DriveMode AutoPilot::DriveToBall()
{
	double speed;
	double rotate;
	double rotateGate;
	int desiredDistance = 280;
	
	while (true) {
		if (stop_thread) return EXIT;
		if ((boost::posix_time::microsec_clock::local_time() - lastUpdate).total_milliseconds() > 1000) return IDLE;

		if (wheels->IsStalled()) return RECOVER_CRASH;
		
		//rotate calculation for ball
		if (lastBallLocation.horizontalAngle > 200){
			rotate = (360 - lastBallLocation.horizontalAngle) * 0.4 + 3;
		}
		else{
			rotate = lastBallLocation.horizontalAngle  * 0.4 + 3;
		}

		//driving commands

		//if ball is close and  center
		if (lastBallLocation.distance < desiredDistance &&
			lastBallLocation.horizontalDev > -10 &&
			lastBallLocation.horizontalDev < 10){
				
				//wheels->Stop();
				coilgun->ToggleTribbler(true);
				return CATCH_BALL;

			}
		//if ball is close and not center
		else if (lastBallLocation.distance <= desiredDistance){
			coilgun->ToggleTribbler(true);
			if (lastBallLocation.horizontalDev < -10) {
				wheels->Rotate(1, rotate);
			}
			else if (lastBallLocation.horizontalDev > 10) {
				wheels->Rotate(0, rotate);
			}
		}
		//if ball is not close 
		else { 
			coilgun->ToggleTribbler(false);
			//speed calculation
			if (lastBallLocation.distance > 700){
				speed = 150;
			}
			else{
				speed = lastBallLocation.distance * 0.33 -77;
			}
			speed = 40;
			//Which way to rotate
			if(lastBallLocation.horizontalAngle > 200){
				wheels->DriveRotate(speed, lastBallLocation.horizontalAngle, -rotate);
			}
			else{
				wheels->DriveRotate(speed, lastBallLocation.horizontalAngle, rotate);
			}
		}

		//check tribbler
		if (ballInTribbler){
			return LOCATE_GATE;
		}
	}
}

DriveMode AutoPilot::CatchBall(){

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime catchStart = time;
	boost::posix_time::time_duration::tick_type catchDuration = (time - catchStart).total_milliseconds();
	//trying to catch ball for 2 seconds
	while (!ballInTribbler && catchDuration < 2000){
		if (stop_thread) return EXIT;
		catchDuration = (time - catchStart).total_milliseconds();
		time = boost::posix_time::microsec_clock::local_time();
		//coilgun->ToggleTribbler(true);//start tribbler
		//wheels->Forward(15);
		wheels->DriveRotate(15, 0, 0);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	if (ballInTribbler){
		return LOCATE_GATE;
	}
	else{
		return LOCATE_BALL;
	}
}

DriveMode AutoPilot::LocateGate() {

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime rotateStart = time;
	boost::posix_time::ptime rotateTime = time;
	//while (true){
		//Search
		while (!gateInSight) {
			coilgun->ToggleTribbler(true);
			if (stop_thread) return EXIT;
			if ((boost::posix_time::microsec_clock::local_time() - lastUpdate).total_milliseconds() > 1000) return IDLE;

			if (wheels->IsStalled()) return RECOVER_CRASH;

			time = boost::posix_time::microsec_clock::local_time();
			if ((time - rotateStart).total_milliseconds() > 10000) { // give up after 10 sec or perhaps go to different search mode
				return IDLE;
			}
			boost::posix_time::time_duration::tick_type rotateDuration = (time - rotateTime).total_milliseconds();
			if (false && rotateDuration >= 500){
				wheels->Stop();
				if (rotateDuration >= 600){
					rotateTime = time; //reset
				}
			}
			else{
				wheels->Rotate(1, 10);
			}
			std::chrono::milliseconds dura(8);
			std::this_thread::sleep_for(dura);
		}
		//Aim
		while (gateInSight){
			if (stop_thread) return EXIT;
			//rotate calculation for gate
			int rotate = 0;
			if (lastGateLocation.horizontalAngle > 200){
				rotate = (360 - lastGateLocation.horizontalAngle) * 0.4 + 3;
			}
			else{
				rotate = lastGateLocation.horizontalAngle  * 0.4 + 3;
			}
			//Turn robot to gate
			if (lastGateLocation.horizontalDev > -30 && lastGateLocation.horizontalDev < 30){
				coilgun->ToggleTribbler(false);
				wheels->Stop();
				std::chrono::milliseconds dura(50);
				std::this_thread::sleep_for(dura);
				coilgun->Kick();
				return LOCATE_BALL;
			}
			else if (lastGateLocation.horizontalDev < -30){
				wheels->Rotate(1, rotate);
			}
			else{
				wheels->Rotate(0, rotate);
			}
		}

	//}

}

DriveMode AutoPilot::RecoverCrash() 
{
	while (wheels->IsStalled()) {
		//Backwards
		wheels->Drive(50, 180);
		std::chrono::milliseconds dura(1000);
		std::this_thread::sleep_for(dura);
		wheels->Stop();
		//Turn a littlebit
		wheels->Rotate(1, 20);
		std::this_thread::sleep_for(dura);
		wheels->Stop();
	}
	return LOCATE_BALL;
}

void AutoPilot::Run()
{

	while (!stop_thread){
		WriteInfoOnScreen();
		switch (driveMode){
		case IDLE:
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			break;
		case LOCATE_BALL:
			driveMode = LocateBall();
			break;
		case DRIVE_TO_BALL:
			driveMode = DriveToBall();
			break;
		case CATCH_BALL:
			driveMode = CatchBall();
			break;
		case LOCATE_GATE:
			driveMode = LocateGate();
			break;
		case RECOVER_CRASH:
			driveMode = RecoverCrash();
			break;
		case EXIT:
			stop_thread = true;
		}
		std::chrono::milliseconds dura(8);
		std::this_thread::sleep_for(dura);
	}
}

void AutoPilot::WriteInfoOnScreen(){
	cv::Mat infoWindow(140, 250, CV_8UC3, cv::Scalar::all(0));
	std::ostringstream oss;
	oss << "State :" << DRIVEMODE_LABELS[driveMode];
	//std::cout << oss.str() << std::endl;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	//std::cout << oss.str() << std::endl;
	oss.str("");
	oss << "Ball visible :" << (ballInSight ? "yes" : "no");
	cv::putText(infoWindow, oss.str(), cv::Point(20, 50), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	//std::cout << oss.str() << std::endl;
	oss.str("");
	oss << "Gate Visible :" << (gateInSight ? "yes" : "no");
	cv::putText(infoWindow, oss.str(), cv::Point(20, 80), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	//std::cout << oss.str() << std::endl;
	oss.str("");
	oss << "Ball in tribbler :" << (ballInTribbler ? "yes" : "no");
	cv::putText(infoWindow, oss.str(), cv::Point(20, 110), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	//std::cout << oss.str() << std::endl;
	cv::imshow("AutoPilot", infoWindow);
	cv::waitKey(1);
	return;
}



AutoPilot::~AutoPilot()
{
	coilgun->ToggleTribbler(false);
	stop_thread = true;
	threads.join_all();

}
