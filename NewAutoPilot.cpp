
#include "NewAutoPilot.h"
#include "coilBoard.h"
#include "Arduino.h"
#include "wheelcontroller.h"
#include <thread>
#define sign(x) (x > 0) - (x < 0)

std::pair<NewDriveMode, DriveInstruction*> NewDriveModes[] = {
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_IDLE, new Idle()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_LOCATE_BALL, new LocateBall()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_BALL, new DriveToBall()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_LOCATE_HOME, new LocateHome()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_DRIVE_TO_HOME, new DriveToHome()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_LOCATE_GATE, new LocateGate()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_AIM_GATE, new AimGate()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_KICK, new Kick()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_CATCH_BALL, new CatchBall()),
	std::pair<NewDriveMode, DriveInstruction*>(DRIVEMODE_RECOVER_CRASH, new RecoverCrash()),

	//	std::pair<STATE, std::string>(STATE_END_OF_GAME, "End of Game") // this is intentionally left out

};


NewAutoPilot::NewAutoPilot(WheelController *wheels, CoilGun *coilgun, Arduino *arduino) :wheels(wheels), coilgun(coilgun), arduino(arduino)
, driveModes(NewDriveModes, NewDriveModes + sizeof(NewDriveModes) / sizeof(NewDriveModes[0]))
{
	curDriveMode = driveModes.find(DRIVEMODE_IDLE);
	stop_thread = false;

	ballInSight = false;
	gateInSight = false;
	homeGateInSight = false;
	ballInTribbler = false;
	sightObstructed = false;
	somethingOnWay = false;

	threads.create_thread(boost::bind(&NewAutoPilot::Run, this));
}

void NewAutoPilot::UpdateState(ObjectPosition *ballLocation, ObjectPosition *gateLocation, bool ballInTribbler, bool sightObstructed, bool somethingOnWay)
{
	boost::mutex::scoped_lock lock(mutex);
	ballInSight = ballLocation != NULL;
	gateInSight = gateLocation != NULL;
	if (ballInSight) lastBallLocation = *ballLocation;
	if (gateInSight) lastGateLocation = *gateLocation;
	this->ballInTribbler = ballInTribbler;
	this->sightObstructed = sightObstructed;
	this->somethingOnWay = somethingOnWay;
	lastUpdate = boost::posix_time::microsec_clock::local_time();

	if (driveMode == DRIVEMODE_IDLE) driveMode = DRIVEMODE_LOCATE_BALL;
}

/*BEGIN Idle*/
void Idle::onEnter(const NewAutoPilot& NewAutoPilot)
{
	idleStart = NewAutoPilot.lastUpdate;
}

NewDriveMode Idle::step(const NewAutoPilot& NewAutoPilot, double dt)
{
	return (idleStart - NewAutoPilot.lastUpdate).total_milliseconds() == 0 ? DRIVEMODE_IDLE : DRIVEMODE_DRIVE_TO_BALL;
}

/*BEGIN LocateBall*/
void LocateBall::onEnter(const NewAutoPilot& NewAutoPilot)
{
	NewAutoPilot.coilgun->ToggleTribbler(false);
	rotateStart = boost::posix_time::microsec_clock::local_time();
}

NewDriveMode LocateBall::step(const NewAutoPilot& NewAutoPilot, double dt)
{
	auto &ballInTribbler = NewAutoPilot.ballInTribbler;
	auto &ballInSight = NewAutoPilot.ballInSight;
	auto &wheels = NewAutoPilot.wheels;
	

	if (ballInTribbler) return DRIVEMODE_LOCATE_GATE;
	if (ballInSight) return DRIVEMODE_DRIVE_TO_BALL;

	//wheels->Stop();
	//return DRIVEMODE_LOCATE_BALL;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration::tick_type rotateDuration = (time - rotateStart).total_milliseconds();

	if (rotateDuration < 5700){
		wheels->Rotate(1, 15);
		return DRIVEMODE_LOCATE_BALL;
	}
	else {
		return DRIVEMODE_DRIVE_TO_HOME;
	}
}
/*BEGIN LocateHome*/
NewDriveMode LocateHome::step(const NewAutoPilot& NewAutoPilot, double dt)
{
	return DRIVEMODE_LOCATE_BALL;
}

/*BEGIN DriveToHome*/
NewDriveMode DriveToHome::step(const NewAutoPilot& NewAutoPilot, double dt)
{
	NewAutoPilot.wheels->Forward(-40);
	std::chrono::milliseconds dura(300);
	std::this_thread::sleep_for(dura);
	NewAutoPilot.wheels->Forward(0);

	return DRIVEMODE_LOCATE_BALL;
}
/*BEGIN DriveToBall*/
void DriveToBall::onEnter(const NewAutoPilot& NewAutoPilot)
{
	NewAutoPilot.coilgun->ToggleTribbler(false);
	start = NewAutoPilot.lastBallLocation;
	target = { 390, 0, 0 };
}

NewDriveMode DriveToBall::step(const NewAutoPilot& NewAutoPilot, double dt)
{
	if (!NewAutoPilot.ballInSight) return DRIVEMODE_LOCATE_BALL;
	if (NewAutoPilot.ballInTribbler) return DRIVEMODE_LOCATE_GATE;

	auto &lastBallLocation = NewAutoPilot.lastBallLocation;
	auto &wheels = NewAutoPilot.wheels;
	auto &coilgun = NewAutoPilot.coilgun;

	//Ball is close and center
	if ((lastBallLocation.distance < target.distance) && abs(lastBallLocation.horizontalDev) <= 8) {
		return DRIVEMODE_CATCH_BALL;
		coilgun->ToggleTribbler(true);
	} 
	//Ball is close and not center
	else if (lastBallLocation.distance < target.distance){
		rotate = abs(lastBallLocation.horizontalAngle)  * 0.46 + 3;
		std::cout << "rotate: " << rotate << std::endl;
		wheels->Rotate(lastBallLocation.horizontalAngle > 0,  -rotate);
		coilgun->ToggleTribbler(true);
	}
	//Ball is far away
	else {
		rotate = abs(lastBallLocation.horizontalAngle)  * 0.46 + 3;
		coilgun->ToggleTribbler(false);
		//speed calculation
		if (lastBallLocation.distance > 700){
			speed = 150;
		}
		else{
			speed = lastBallLocation.distance * 0.474 - 182;
			
		}
		//speed = 30;
		wheels->DriveRotate(speed, -lastBallLocation.horizontalAngle, lastBallLocation.horizontalAngle > 0?-rotate:rotate);
	}
	return DRIVEMODE_DRIVE_TO_BALL;
}
/*BEGIN CatchBall*/
void CatchBall::onEnter(const NewAutoPilot& NewAutoPilot)
{
	NewAutoPilot.coilgun->ToggleTribbler(true);
	catchStart = boost::posix_time::microsec_clock::local_time();
	NewAutoPilot.wheels->Stop();
}
void CatchBall::onExit(const NewAutoPilot& NewAutoPilot)
{
	//NewAutoPilot.coilgun->ToggleTribbler(false);
}
NewDriveMode CatchBall::step(const NewAutoPilot& NewAutoPilot, double dt)
{
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration::tick_type catchDuration = (time - catchStart).total_milliseconds();
	auto &lastBallLocation = NewAutoPilot.lastBallLocation;
	auto &wheels = NewAutoPilot.wheels;
	auto &coilgun = NewAutoPilot.coilgun;
	//std::cout << catchDuration << std::endl;
	if (NewAutoPilot.ballInTribbler) {
		return DRIVEMODE_LOCATE_GATE;
	}
	else if (catchDuration > 2000) { //trying to catch ball for 2 seconds
		return DRIVEMODE_LOCATE_BALL;
	}
	else {
		NewAutoPilot.wheels->DriveRotate(40, 0, 0);
	}
	return DRIVEMODE_CATCH_BALL;
}
/*END CatchBall*/

/*BEGIN LocateGate*/
NewDriveMode LocateGate::step(const NewAutoPilot& NewAutoPilot, double dt)
{
	auto &gateInSight = NewAutoPilot.gateInSight;
	auto &coilgun = NewAutoPilot.coilgun;
	auto &ballInTribbler = NewAutoPilot.ballInTribbler;
	auto &wheels = NewAutoPilot.wheels;
	auto &sightObstructed = NewAutoPilot.sightObstructed;
	auto &lastGateLocation = NewAutoPilot.lastGateLocation;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();

	if (!ballInTribbler) return DRIVEMODE_LOCATE_BALL;
	if (gateInSight) return DRIVEMODE_AIM_GATE;


	wheels->Rotate(0, 30);

	return DRIVEMODE_LOCATE_GATE;
}
/*BEGIN AimGate*/
NewDriveMode AimGate::step(const NewAutoPilot& NewAutoPilot, double dt)
{

	auto &gateInSight = NewAutoPilot.gateInSight;
	auto &coilgun = NewAutoPilot.coilgun;
	auto &ballInTribbler = NewAutoPilot.ballInTribbler;
	auto &wheels = NewAutoPilot.wheels;
	auto &sightObstructed = NewAutoPilot.sightObstructed;
	auto &lastGateLocation = NewAutoPilot.lastGateLocation;


	if (!ballInTribbler) return DRIVEMODE_LOCATE_BALL;
	if (!gateInSight) return DRIVEMODE_LOCATE_GATE;


	//Turn robot to gate
	if (abs(lastGateLocation.horizontalAngle) < 20) {
		if (sightObstructed) { //then move sideways
			wheels->Drive(50, -90);
			std::chrono::milliseconds dura(400);
			std::this_thread::sleep_for(dura);
		}
		else {
			return DRIVEMODE_KICK;
		}
	}
	else {
		//rotate calculation for gate
		int rotate = lastGateLocation.horizontalAngle  * 0.4 + 3;
		wheels->Rotate(0, rotate);
	}
	return DRIVEMODE_AIM_GATE;

}

/*BEGIN Kick*/
NewDriveMode Kick::step(const NewAutoPilot& NewAutoPilot, double dt)
{
	NewAutoPilot.coilgun->ToggleTribbler(false);
	NewAutoPilot.wheels->Stop();
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	NewAutoPilot.coilgun->Kick();
	std::this_thread::sleep_for(std::chrono::milliseconds(500)); //half second wait.
	return DRIVEMODE_LOCATE_BALL;

}
void Kick::onEnter(const NewAutoPilot& NewAutoPilot)
{
	NewAutoPilot.coilgun->ToggleTribbler(false);
}

/*BEGIN RecoverCrash*/
NewDriveMode RecoverCrash::step(const NewAutoPilot& NewAutoPilot, double dt)
{
	double velocity2 = 0, direction2 = 0, rotate2 = 0;
	auto targetSpeed = NewAutoPilot.wheels->GetTargetSpeed();

	//Backwards
	NewAutoPilot.wheels->Drive(-targetSpeed.velocity, 180 - targetSpeed.heading);
	std::chrono::milliseconds dura(1000);
	std::this_thread::sleep_for(dura);
	NewAutoPilot.wheels->Stop();

	return DRIVEMODE_LOCATE_BALL;
}

void NewAutoPilot::Run()
{
	boost::posix_time::ptime lastStep = boost::posix_time::microsec_clock::local_time();
	NewDriveMode newMode = curDriveMode->first;
	curDriveMode->second->onEnter(*this);
	while (!stop_thread){
		if ((boost::posix_time::microsec_clock::local_time() - lastUpdate).total_milliseconds() > 1000) {
			newMode = DRIVEMODE_IDLE;
		}
		//else if (wheels->IsStalled() && curDriveMode->first != DRIVEMODE_RECOVER_CRASH){
		//	newMode = DRIVEMODE_RECOVER_CRASH;
		//}
		else {
			boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
			boost::posix_time::time_duration::tick_type dt = (time - lastStep).total_milliseconds();
			newMode = curDriveMode->second->step(*this, dt);
		}
		auto old = curDriveMode;
		if (newMode != curDriveMode->first){
			boost::mutex::scoped_lock lock(mutex);
			curDriveMode->second->onExit(*this);
			//wheels->Stop();
			curDriveMode = driveModes.find(newMode);
			if (curDriveMode == driveModes.end()) {
				std::cout << "Invalid drive mode from :" << old->second->name << ", reverting to locate_ball" << std::endl;
				curDriveMode = driveModes.find(DRIVEMODE_LOCATE_BALL);;
			}
			std::cout << "state change :" << old->second->name << " ->" << curDriveMode->second->name << std::endl;

			curDriveMode->second->onEnter(*this);


			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
	std::cout << "NewAutoPilot stoping" << std::endl;

}

std::string NewAutoPilot::GetDebugInfo(){
	std::ostringstream oss;
	boost::mutex::scoped_lock lock(mutex);
	oss << "[NewAutoPilot] State: " << curDriveMode->second->name;

	return oss.str();
}

void NewAutoPilot::WriteInfoOnScreen(){
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
	cv::imshow("NewAutoPilot", infoWindow);
	cv::waitKey(1);
	return;
}



NewAutoPilot::~NewAutoPilot()
{
	stop_thread = true;
	threads.join_all();
	for (auto &mode : driveModes){
		delete mode.second;
	}
	//coilgun->ToggleTribbler(false);

}
