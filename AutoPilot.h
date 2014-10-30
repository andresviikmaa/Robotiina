#pragma once
#include "types.h"
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>

class CoilGun;
class WheelController;
enum DriveMode {
	IDLE = 0,
	LOCATE_BALL,
	DRIVE_TO_BALL,
	LOCATE_GATE,
	RECOVER_CRASH,
	EXIT
};
class AutoPilot
{
private:
	WheelController *wheels;
	CoilGun *coilgun;
	ObjectPosition lastBallLocation;
	ObjectPosition lastGateLocation;
	bool ballInSight = false;
	bool gateInSight = false;
	bool ballInTribbler = false;

	boost::atomic<bool> stop_thread;
	boost::atomic<bool> drive;
	boost::thread_group threads;
	boost::mutex mutex;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime lastStep = time; 
	DriveMode driveMode = LOCATE_BALL;
protected:
	DriveMode DriveToBall();
	DriveMode LocateBall();
	DriveMode LocateGate();
	DriveMode RecoverCrash();
	void Step();
public:
	AutoPilot(WheelController *wheels, CoilGun *coilgun);
	void UpdateState(ObjectPosition *ballLocation, ObjectPosition *gateLocation);
	void Run();
	~AutoPilot();
};

