
#include "AutoPilot.h"
#include "coilBoard.h"
#include "Audrino.h"
#include "wheelcontroller.h"
#include <thread>

std::pair<DriveMode, DriveInstruction*> DriveModes[] = {
	std::pair<DriveMode, DriveInstruction*>(IDLE, new Idle()),
	std::pair<DriveMode, DriveInstruction*>(LOCATE_BALL, new LocateBall()),
	std::pair<DriveMode, DriveInstruction*>(DRIVE_TO_BALL, new DriveToBall()),
	std::pair<DriveMode, DriveInstruction*>(LOCATE_HOME, new LocateHome()),
	std::pair<DriveMode, DriveInstruction*>(DRIVE_TO_HOME, new DriveToHome()),
	std::pair<DriveMode, DriveInstruction*>(LOCATE_GATE, new LocateGate()),
	std::pair<DriveMode, DriveInstruction*>(AIM_GATE, new AimGate()),
	std::pair<DriveMode, DriveInstruction*>(KICK, new Kick()),
	std::pair<DriveMode, DriveInstruction*>(CATCH_BALL, new CatchBall()),
	std::pair<DriveMode, DriveInstruction*>(RECOVER_CRASH, new RecoverCrash()),

	//	std::pair<STATE, std::string>(STATE_END_OF_GAME, "End of Game") // this is intentionally left out

};


AutoPilot::AutoPilot(WheelController *wheels, CoilGun *coilgun, Audrino *audrino) :wheels(wheels), coilgun(coilgun), audrino(audrino)
, driveModes(DriveModes, DriveModes + sizeof(DriveModes) / sizeof(DriveModes[0]))
{
	curDriveMode = driveModes.find(IDLE);
	stop_thread = false;
	threads.create_thread(boost::bind(&AutoPilot::Run, this));
}

void AutoPilot::UpdateState(ObjectPosition *ballLocation, ObjectPosition *gateLocation, bool sightObstructed)
{
	boost::mutex::scoped_lock lock(mutex);
	ballInSight = ballLocation != NULL;
	gateInSight = gateLocation != NULL;
	if (ballInSight) lastBallLocation = *ballLocation;
	if (gateInSight) lastGateLocation = *gateLocation;
	ballInTribbler =  coilgun->BallInTribbler();
	lastUpdate = boost::posix_time::microsec_clock::local_time();
	sonars = audrino->GetSonarReadings();
	this->sightObstructed = sightObstructed;

	if (driveMode == IDLE) driveMode = LOCATE_BALL;
}

/*BEGIN Idle*/
void Idle::onEnter(const AutoPilot& autoPilot)
{
	idleStart = autoPilot.lastUpdate;
}

DriveMode Idle::step(const AutoPilot& autoPilot, double dt)
{
	std::cout << "idle: " << (idleStart - autoPilot.lastUpdate).total_milliseconds() << std::endl;
	return (idleStart - autoPilot.lastUpdate).total_milliseconds() == 0 ? IDLE : DRIVE_TO_BALL;
}

/*BEGIN LocateBall*/
void LocateBall::onEnter(const AutoPilot& autoPilot)
{
	rotateStart = boost::posix_time::microsec_clock::local_time();
}

DriveMode LocateBall::step(const AutoPilot& autoPilot, double dt)
{
	auto &ballInTribbler = autoPilot.ballInTribbler;
	auto &ballInSight = autoPilot.ballInSight;
	auto &wheels = autoPilot.wheels;

	if (ballInTribbler) return LOCATE_GATE;
	if (ballInSight) return DRIVE_TO_BALL;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration::tick_type rotateDuration = (time - rotateStart).total_milliseconds();

	if (rotateDuration < 5700){
		wheels->Rotate(1,15);
		return LOCATE_BALL;
	}
	else {				
		return DRIVE_TO_HOME;
	}
}
/*BEGIN LocateHome*/
DriveMode LocateHome::step(const AutoPilot& autoPilot, double dt)
{
	return LOCATE_BALL;
}

/*BEGIN DriveToHome*/
DriveMode DriveToHome::step(const AutoPilot& autoPilot, double dt)
{
	autoPilot.wheels->Forward(-70);
	std::chrono::milliseconds dura(1000);
	std::this_thread::sleep_for(dura);

	return LOCATE_BALL;
}
/*BEGIN DriveToBall*/
void DriveToBall::onEnter(const AutoPilot& autoPilot)
{
	autoPilot.coilgun->ToggleTribbler(false);
	start = autoPilot.lastBallLocation;
	target = { 270, 0, 0};
}

DriveMode DriveToBall::step(const AutoPilot& autoPilot, double dt)
{
	if (!autoPilot.ballInSight) return LOCATE_BALL;
	if (autoPilot.ballInTribbler) return LOCATE_GATE;

	auto &lastBallLocation = autoPilot.lastBallLocation;
	auto &wheels = autoPilot.wheels;

	if (lastBallLocation.distance < target.distance
		&& abs(lastBallLocation.horizontalDev) < target.horizontalDev ) {
		return CATCH_BALL;
	}


	//rotate calculation for ball
	rotate = lastBallLocation.horizontalAngle  * 0.4 + 3;
	
	if (lastBallLocation.distance <= desiredDistance){
			//coilgun->ToggleTribbler(true);
			if (lastBallLocation.horizontalDev < -10) {
				wheels->Rotate(1, rotate);
			}
			else if (lastBallLocation.horizontalDev > 10) {
				wheels->Rotate(0, rotate);
			}
		}
		//if ball is not close 
		else { 
			//coilgun->ToggleTribbler(false);
			//speed calculation
			if (lastBallLocation.distance > 700){
				speed = 150;
			}
			else{
				speed = lastBallLocation.distance * 0.33 -77;
			}
			wheels->DriveRotate(speed, lastBallLocation.horizontalAngle, rotate);
		}
		return DRIVE_TO_BALL;

	
}
/*BEGIN CatchBall*/
void CatchBall::onEnter(const AutoPilot& autoPilot)
{
	autoPilot.coilgun->ToggleTribbler(true);
	catchStart = boost::posix_time::microsec_clock::local_time();
}
void CatchBall::onExit(const AutoPilot& autoPilot)
{
	//autoPilot.coilgun->ToggleTribbler(false);
}
DriveMode CatchBall::step(const AutoPilot& autoPilot, double dt)
{
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration::tick_type catchDuration = (time - catchStart).total_milliseconds();

	if (autoPilot.ballInTribbler) {
		return LOCATE_GATE;
	}
	else if (catchDuration > 2000) { //trying to catch ball for 2 seconds
		return LOCATE_BALL;

	}
	else {
		autoPilot.wheels->DriveRotate(40, 0, 0);
		//std::this_thread::sleep_for(std::chrono::milliseconds(100));
		return CATCH_BALL;
	}
}
/*END CatchBall*/

/*BEGIN LocateGate*/
DriveMode LocateGate::step(const AutoPilot& autoPilot, double dt)
{
	auto &gateInSight = autoPilot.gateInSight;
	auto &coilgun = autoPilot.coilgun;
	auto &ballInTribbler = autoPilot.ballInTribbler;
	auto &wheels = autoPilot.wheels;
	auto &sightObstructed = autoPilot.sightObstructed;
	auto &lastGateLocation = autoPilot.lastGateLocation;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();

	if (!ballInTribbler) return LOCATE_BALL;
	if (gateInSight) return AIM_GATE;
	
	
	wheels->Rotate(0, 15);
	
	return LOCATE_GATE;
}
/*BEGIN AimGate*/
DriveMode AimGate::step(const AutoPilot& autoPilot, double dt)
{

	auto &gateInSight = autoPilot.gateInSight;
	auto &coilgun = autoPilot.coilgun;
	auto &ballInTribbler = autoPilot.ballInTribbler;
	auto &wheels = autoPilot.wheels;
	auto &sightObstructed = autoPilot.sightObstructed;
	auto &lastGateLocation = autoPilot.lastGateLocation;


	if (!ballInTribbler) return LOCATE_BALL;
	if (!gateInSight) return LOCATE_GATE;


		//Turn robot to gate
		if (abs(lastGateLocation.horizontalDev) < 30) {
			if (sightObstructed) { //then move sideways
				wheels->Drive(50, 270);
				std::chrono::milliseconds dura(400);
				std::this_thread::sleep_for(dura);
				return LOCATE_GATE;
			}
			else {
				return KICK;
			}
		}
		else {
			//rotate calculation for gate
			int rotate = lastGateLocation.horizontalAngle  * 1.25 + 5;
			wheels->Rotate(0, rotate);
		}
	

	//}
	return LOCATE_BALL;
}

/*BEGIN Kick*/
DriveMode Kick::step(const AutoPilot& autoPilot, double dt)
{
	autoPilot.coilgun->ToggleTribbler(false);
	autoPilot.wheels->Stop();
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	autoPilot.coilgun->Kick();
	std::this_thread::sleep_for(std::chrono::milliseconds(500)); //half second wait.
	return LOCATE_BALL;

}

/*BEGIN RecoverCrash*/
DriveMode RecoverCrash::step(const AutoPilot& autoPilot, double dt)
{
	double velocity2 = 0, direction2 = 0, rotate2 = 0;
	autoPilot.wheels->GetTargetSpeed(velocity2, direction2, rotate2);

	//Backwards
	autoPilot.wheels->Drive(-velocity2, 180 - direction2);
	std::chrono::milliseconds dura(1000);
	std::this_thread::sleep_for(dura);
	autoPilot.wheels->Stop();
	
	return LOCATE_BALL;
}

void AutoPilot::Run()
{
	boost::posix_time::ptime lastStep = boost::posix_time::microsec_clock::local_time();
	DriveMode newMode = curDriveMode->first;
	curDriveMode->second->onEnter(*this);
	while (!stop_thread){
		if ((boost::posix_time::microsec_clock::local_time() - lastUpdate).total_milliseconds() > 1000) {
			newMode = IDLE;
		}
		else if (wheels->IsStalled() && curDriveMode->first != RECOVER_CRASH){
			newMode = RECOVER_CRASH;
		}
		else {
			boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
			boost::posix_time::time_duration::tick_type dt = (time - lastStep).total_milliseconds();
			newMode = curDriveMode->second->step(*this, dt);
		}

		if (newMode != curDriveMode->first){
			boost::mutex::scoped_lock lock(mutex);
			curDriveMode->second->onExit(*this);
			//wheels->Stop();
			curDriveMode = driveModes.find(newMode);
			curDriveMode->second->onEnter(*this);

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		/*
		WriteInfoOnScreen();
		switch (driveMode){
		case IDLE:
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			wheels->Stop();
			coilgun->ToggleTribbler(false);
			break;
		case LOCATE_BALL:
			driveMode = LocateBall();
			break;
		case DRIVE_TO_HOME:
			driveMode = DriveToHome();
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
		case LOCATE_HOME:
			driveMode = LocateHome();
			break;
		case RECOVER_CRASH:
			driveMode = RecoverCrash();
			break;
		case EXIT:
			stop_thread = true;
		}
		std::chrono::milliseconds dura(8);
		std::this_thread::sleep_for(dura);
		*/
	}
}

std::string AutoPilot::GetDebugInfo(){
	std::ostringstream oss;
	boost::mutex::scoped_lock lock(mutex);
	oss << "[Autopilot] State: " << curDriveMode->second->name;
	oss << ", Ball visible: " << (ballInSight ? "yes" : "no");
	oss << ", Gate Visible: " << (gateInSight ? "yes" : "no");
	oss << ", Ball in tribbler: " << (ballInTribbler ? "yes" : "no");
	oss << ", Sight free: " << (!sightObstructed ? "yes" : "no");
	oss << "|[Autopilot] Ball Pos: (" << lastBallLocation.distance << "," << lastBallLocation.horizontalAngle << "," << lastBallLocation.horizontalDev << ")";
	oss << "Gate Pos: (" << lastBallLocation.distance << "," << lastBallLocation.horizontalAngle << "," << lastBallLocation.horizontalDev << ")";

	return oss.str();
}

void AutoPilot::WriteInfoOnScreen(){
	cv::Mat infoWindow(140, 250, CV_8UC3, cv::Scalar::all(0));
	std::ostringstream oss;
	oss << "State: " << curDriveMode->second->name;
	//std::cout << oss.str() << std::endl;
	cv::putText(infoWindow, oss.str(), cv::Point(20, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	//std::cout << oss.str() << std::endl;
	oss.str("");
	oss << "Ball visible: " << (ballInSight ? "yes" : "no");
	cv::putText(infoWindow, oss.str(), cv::Point(20, 50), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	//std::cout << oss.str() << std::endl;
	oss.str("");
	oss << "Gate Visible: " << (gateInSight ? "yes" : "no");
	cv::putText(infoWindow, oss.str(), cv::Point(20, 80), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	//std::cout << oss.str() << std::endl;
	oss.str("");
	oss << "Ball in tribbler: " << (ballInTribbler ? "yes" : "no");
	cv::putText(infoWindow, oss.str(), cv::Point(20, 110), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
	//std::cout << oss.str() << std::endl;
	cv::imshow("AutoPilot", infoWindow);
	cv::waitKey(1);
	return;
}



AutoPilot::~AutoPilot()
{
	stop_thread = true;
	threads.join_all();
	for (auto &mode : driveModes){
		delete mode.second;
	}
	coilgun->ToggleTribbler(false);

}
