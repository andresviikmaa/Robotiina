#pragma once
#include "types.h"
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>

class CoilGun;
class WheelController;
class Audrino;
enum DriveMode {
	IDLE = 0,
	LOCATE_BALL,
	DRIVE_TO_BALL,
	LOCATE_HOME,
	DRIVE_TO_HOME,
	CATCH_BALL,
	LOCATE_GATE,
	AIM_GATE,
	KICK,
	RECOVER_CRASH,
	EXIT
};
class DriveInstruction
{
	friend class AutoPilot;
	const std::string name;
public:
	DriveInstruction(const std::string name) : name(name){};
	virtual void onEnter(const AutoPilot& autoPilot){};
	virtual DriveMode step(const AutoPilot& autoPilot, double dt) = 0;
	virtual void onExit(const AutoPilot& autoPilot){};

};
class Idle : public DriveInstruction
{
private:
	boost::posix_time::ptime idleStart; 
public:
	Idle() : DriveInstruction("IDLE"){};
	virtual void onEnter(const AutoPilot& autoPilot);
	virtual DriveMode step(const AutoPilot& autoPilot, double dt);
};

class LocateBall : public DriveInstruction
{
private:
	boost::posix_time::ptime rotateStart;
public:
	LocateBall() : DriveInstruction("LOCATE_BALL"){};
	virtual void onEnter(const AutoPilot& autoPilot);
	virtual DriveMode step(const AutoPilot& autoPilot, double dt);
};

class LocateHome : public DriveInstruction
{
public:
	LocateHome() : DriveInstruction("LOCATE_HOME"){};
	virtual DriveMode step(const AutoPilot& autoPilot, double dt);
};

class DriveToHome : public DriveInstruction
{
public:
	DriveToHome() : DriveInstruction("DRIVE_TO_HOME"){};
	virtual DriveMode step(const AutoPilot& autoPilot, double dt);
};


class CatchBall : public DriveInstruction
{
private:
	boost::posix_time::ptime catchStart;
public:
	CatchBall() : DriveInstruction("CATCH_BALL"){};
	virtual void onEnter(const AutoPilot& autoPilot);
	virtual void onExit(const AutoPilot& autoPilot);
	virtual DriveMode step(const AutoPilot& autoPilot, double dt);
};

class DriveToBall : public DriveInstruction
{
private:
	ObjectPosition start;
	ObjectPosition target;
	double speed;
	double rotate;
	double rotateGate;
	int desiredDistance = 270;
public:
	DriveToBall() : DriveInstruction("DRIVE_TO_BALL"){};
	virtual void onEnter(const AutoPilot& autoPilot);
	virtual DriveMode step(const AutoPilot& autoPilot, double dt);
};


class LocateGate : public DriveInstruction
{
public:
	LocateGate() : DriveInstruction("LOCATE_GATE"){};
	virtual DriveMode step(const AutoPilot& autoPilot, double dt);
};

class AimGate : public DriveInstruction
{
public:
	AimGate() : DriveInstruction("AIM_GATE"){};
	virtual DriveMode step(const AutoPilot& autoPilot, double dt);
};

class Kick : public DriveInstruction
{
public:
	Kick() : DriveInstruction("KICK"){};
	virtual DriveMode step(const AutoPilot& autoPilot, double dt);
};

class RecoverCrash : public DriveInstruction
{
public:
	RecoverCrash() : DriveInstruction("RECOVER_CRASH"){};
	virtual DriveMode step(const AutoPilot& autoPilot, double dt);
};


class AutoPilot
{
	friend class Idle;
	friend class DriveToBall;
	friend class CatchBall;
	friend class LocateBall;
	friend class LocateHome;
	friend class DriveToHome;
	friend class LocateGate;
	friend class AimGate;
	friend class Kick;
	friend class RecoverCrash;
private:
	std::map<DriveMode, DriveInstruction*> driveModes;
	std::map<DriveMode, DriveInstruction*>::iterator curDriveMode;
	WheelController *wheels;
	CoilGun *coilgun;
	Audrino *audrino;
	ObjectPosition lastBallLocation;
	ObjectPosition lastGateLocation;
	ObjectPosition lastHomeGateLocation;
	volatile bool ballInSight = false;
	volatile bool gateInSight = false;
	volatile bool homeGateInSight = false;
	volatile bool ballInTribbler = false;
	volatile bool sightObstructed = false;
	cv::Point3i sonars{ 100, 100, 100 };


	boost::atomic<bool> stop_thread;
	boost::atomic<bool> drive;
	boost::thread_group threads;
	boost::mutex mutex;
	boost::posix_time::ptime rotateTime = time;

	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime lastUpdate = time; 
	DriveMode lastDriveMode = IDLE;
	DriveMode driveMode = IDLE;

protected:
//	DriveMode DriveToBall();
	DriveMode LocateBall();
	DriveMode CatchBall();
	DriveMode LocateGate();
	DriveMode LocateHome();
	DriveMode DriveToHome();
	DriveMode RecoverCrash();
	void Step();
	void WriteInfoOnScreen();
public:
	AutoPilot(WheelController *wheels, CoilGun *coilgun, Audrino *audrino);
	void UpdateState(ObjectPosition *ballLocation, ObjectPosition *gateLocation, bool sightObstructed);
	void Run();
	~AutoPilot();
	std::string GetDebugInfo();
};

