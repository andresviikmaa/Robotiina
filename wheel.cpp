#include "wheel.h"
#include <chrono>
#include <thread>

BasicWheel::BasicWheel()
{
	stall = false;
	stop_thread = false;
	threads.create_thread(boost::bind(&BasicWheel::Run, this));
};

void BasicWheel::Run()
{
	while (!stop_thread){
		time = boost::posix_time::microsec_clock::local_time();
		UpdateSpeed();
		CheckStall();
		lastStep = time;
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // do not poll serial to fast

	}
}

void BasicWheel::CheckStall()
{
	int diff = abs(actual_speed - target_speed);
	boost::posix_time::time_duration::tick_type stallDuration = (time - stallTime).total_milliseconds();
	if (diff > 10){
		std::cout << "diff: " << diff << std::endl;

		if (stallDuration > 400){ stall = true; }
	}
	else{
		stallTime = time;
		stall = false;
	}

}


BasicWheel::~BasicWheel()
{
	stop_thread = true;
	threads.join_all();
}

void SoftwareWheel::UpdateSpeed()
{
	double dt = (double)(time - lastStep).total_milliseconds() / 1000.0;
	if (dt < 0.0000001) return;

	double dv = target_speed - actual_speed;
	double sign = (dv > 0) - (dv < 0);
	double acc = dv / dt;
	acc = sign * std::min(abs(acc), max_acceleration);

	if (false && rand() % 1000 > 995) { // 0.5% probability to stall
		actual_speed = 0;
	}
	else {
		assert(actual_speed > -1000);
		actual_speed += acc * dt;
		assert(actual_speed > -1000);
	}
};

void SerialWheel::UpdateSpeed()
{
	boost::mutex::scoped_lock lock(mutex);
	if (update_speed){
		std::ostringstream oss;
		oss << "sd" << target_speed << "\n";
		writeString(oss.str());
		update_speed = false;
	}
	writeString("s\n");
	actual_speed = atoi(readLine().substr(3).c_str());

};
