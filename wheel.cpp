#include "wheel.h"
#include <chrono>
#include <thread>

BasicWheel::BasicWheel()
{
	stall = false;
	stop_thread = false;
	update_speed = false;
	target_speed = 0;
	actual_speed = 0;
}
void BasicWheel::Start()
{
	threads.create_thread(boost::bind(&BasicWheel::Run, this));
};
void BasicWheel::Stop()
{
	stop_thread = true;
	threads.join_all();
};
void BasicWheel::SetSpeed(int given_speed) {
	//boost::mutex::scoped_lock lock(mutex);
	target_speed = given_speed;
	update_speed = true;
};

void BasicWheel::Run()
{
	while (!stop_thread){
		time = boost::posix_time::microsec_clock::local_time();
		{ // new scope for locking
			//boost::mutex::scoped_lock lock(mutex);
			UpdateSpeed();
			CheckStall();
			//CalculateDistanceTraveled();
		}
		lastStep = time;
		// speed update interval is 62.5Hz
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // do not poll serial to fast

	}
}

void BasicWheel::CheckStall()
{
	int diff = abs(actual_speed - target_speed);
	boost::posix_time::time_duration::tick_type stallDuration = (time - stallTime).total_milliseconds();
	if (diff > 30){
		if (!stall && stallDuration > 500){ 
			std::cout << "stalled, diff: " << diff << " = " << actual_speed  << " != " << target_speed << std::endl;
			stall = true; 
		}
	}
	else{
		stallTime = time;
		stall = false;
	}

}

int BasicWheel::GetDistanceTraveled(bool reset) 
{
	{ // new scope for locking
		//boost::mutex::scoped_lock lock(mutex);
		return distance_traveled;
		if (reset) distance_traveled = 0;
	}

}

BasicWheel::~BasicWheel()
{
	Stop();
	assert(stop_thread);
}

void SoftwareWheel::UpdateSpeed()
{
	last_speed = actual_speed;
	double dt = (double)(time - lastStep).total_milliseconds() / 1000.0;
	if (dt < 0.0000001) return;

	if ((time - lastUpdate).total_milliseconds() > stop_time) { // die out if no update
		actual_speed = 0;
		return;
	}

	double dv = target_speed - actual_speed;
	double sign = (dv > 0) - (dv < 0);
	double acc = dv / dt;
	acc = sign * std::min((int)abs(acc), max_acceleration);

	if (false && rand() % 1000 > 995) { // 0.5% probability to stall
		stallStart = time;
		actual_speed = 0;
	}
	else if ((time - stallStart).total_milliseconds() < 600) {
		;// still stalled
	}
	else {
		actual_speed += acc * dt;
	}
};

void SerialWheel::UpdateSpeed()
{
	last_speed = actual_speed;

	if (update_speed){
		lastUpdate = boost::posix_time::microsec_clock::local_time();
		std::ostringstream oss;
		oss << "sd" << target_speed << "\n";
		writeString(oss.str());
		update_speed = false;
	}
	writeString("s\n");
	std::string line = readLine();
	if( line.length() > 0 )
		actual_speed = atoi(line.substr(3).c_str());
	else
		actual_speed = 0;

};
