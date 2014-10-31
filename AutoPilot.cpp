
#include "AutoPilot.h"
#include "coilBoard.h"
#include "wheelcontroller.h"
#include <thread>

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
	lastUpdate = boost::posix_time::microsec_clock::local_time();
	if (driveMode == IDLE) driveMode = LOCATE_BALL;
}

DriveMode AutoPilot::DriveToBall()
{
	double speed;
	double rotate;
	int desiredDistance = 210;
	while (!coilgun->BallInTribbler()) {
		boost::mutex::scoped_lock lock(mutex);
		if (stop_thread) return EXIT;
		if ((boost::posix_time::microsec_clock::local_time() - lastUpdate).total_milliseconds() > 1000) return IDLE;
		if (!ballInSight) return LOCATE_BALL;
		if (wheels->IsStalled()) return RECOVER_CRASH;

		/* END remove when IR sensor is working */
		//if ball is close and center i.e. Ball in tribbler
		if (lastBallLocation.distance < desiredDistance && (lastBallLocation.horizontalDev > -10 && lastBallLocation.horizontalDev < 10)){
			break; // ballInTribbler
		}
		/* END remove when IR sensor is working */

		//rotate calculation
		if (lastBallLocation.horizontalAngle > 200){
			rotate = (360 - lastBallLocation.horizontalAngle) * 2.5;
		}
		else{
			rotate = lastBallLocation.horizontalAngle * 2.5;
		}

		if (lastBallLocation.distance < desiredDistance) { //if ball is close but not center
			coilgun->ToggleTribbler(true);//start tribbler
			if (lastBallLocation.horizontalDev < -10) {
				wheels->Rotate(0, rotate);
			}
			else if (lastBallLocation.horizontalDev > 10) {
				wheels->Rotate(1, rotate);
			}
			// else do nothing, and hope that ball is in tribbler
			return LOCATE_GATE;
		}
		else { //if ball is not close 
			coilgun->ToggleTribbler(false);
			//speed calculation
			if (lastBallLocation.distance > 700){
				speed = 150;
			}
			else{
				speed = lastBallLocation.distance * 0.26 - 32;
			}

			//driving commands
			if (lastBallLocation.horizontalDev > -20 && lastBallLocation.horizontalDev < 20){
				wheels->DriveRotate(speed, lastBallLocation.horizontalAngle, 0);
			}
			else if (lastBallLocation.horizontalDev >= 20){
				wheels->DriveRotate(speed, lastBallLocation.horizontalAngle, rotate);
			}
			else{
				wheels->DriveRotate(speed, lastBallLocation.horizontalAngle, -rotate);
			}

		}
	}
	return LOCATE_GATE;
}
/*
	No ball in sight
*/
DriveMode AutoPilot::LocateBall() { 
	if (ballInSight) return DRIVE_TO_BALL;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime rotateStart = time;
	boost::posix_time::ptime rotateTime = time;
	while (!ballInSight) {
		if (stop_thread) return EXIT;
		if ((boost::posix_time::microsec_clock::local_time() - lastUpdate).total_milliseconds() > 1000) return IDLE;

		if (wheels->IsStalled()) return RECOVER_CRASH;

		time = boost::posix_time::microsec_clock::local_time();
		if ((time - rotateStart).total_milliseconds() > 10000) { // give up after 10 sec or perhaps go to different search mode
			return IDLE;
		}
		boost::posix_time::time_duration::tick_type rotateDuration = (time - rotateTime).total_milliseconds();
		if (rotateDuration >= 500){
			wheels->Stop();
			if (rotateDuration >= 600){
				rotateTime = time; //reset
			}
		}
		else{
			wheels->Rotate(1, 30);
		}
	
		
	}
	return DRIVE_TO_BALL;
}

DriveMode AutoPilot::LocateGate() {

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime rotateStart = time;
	boost::posix_time::ptime rotateTime = time;
	while (!gateInSight) {
		if (stop_thread) return EXIT;
		if ((boost::posix_time::microsec_clock::local_time() - lastUpdate).total_milliseconds() > 1000) return IDLE;

		if (!coilgun->BallInTribbler()) return DRIVE_TO_BALL;
		if (wheels->IsStalled()) return RECOVER_CRASH;

		time = boost::posix_time::microsec_clock::local_time();
		if ((time - rotateStart).total_milliseconds() > 10000) { // give up after 10 sec or perhaps go to different search mode
			return IDLE;
		}
		boost::posix_time::time_duration::tick_type rotateDuration = (time - rotateTime).total_milliseconds();
		if (rotateDuration >= 500){
			wheels->Stop();
			if (rotateDuration >= 600){
				rotateTime = time; //reset
			}
		}
		else{
			wheels->Rotate(1, 30);
		}
	}
	// gate in sight
	coilgun->Kick();
	coilgun->ToggleTribbler(false);
	return DRIVE_TO_BALL;
}

DriveMode AutoPilot::RecoverCrash() 
{
	while (!wheels->IsStalled()) {
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
		case LOCATE_GATE:
			driveMode = LocateGate();
			break;
		case RECOVER_CRASH:
			driveMode = RecoverCrash();
			break;
		case EXIT:
			stop_thread = true;
		}
	}
}

void AutoPilot::WriteInfoOnScreen(){
	cv::Mat infoWindow(100, 250, CV_8UC3, cv::Scalar::all(0));
	std::ostringstream oss;
	oss << "State :" << driveMode;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	oss.str("");
	oss << "Ball visible :" << (ballInSight ? "yes" : "no");
	cv::putText(infoWindow, oss.str(), cv::Point(20, 50), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	oss.str("");
	oss << "Gate Visible :" << (gateInSight ? "yes" : "no");
	cv::putText(infoWindow, oss.str(), cv::Point(20, 80), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	cv::imshow("AutoPilot", infoWindow);
	cv::waitKey(1);
	return;
}



AutoPilot::~AutoPilot()
{
	stop_thread = true;
	threads.join_all();
}
