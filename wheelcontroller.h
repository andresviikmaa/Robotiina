#pragma once

#include "wheel.h"
#include "types.h"
#define EMUATE_WHEEL_RESPONSE

#ifdef EMUATE_WHEEL_RESPONSE
#include <boost/thread/thread.hpp>
#include "wheelemulator.h"
#endif

class WheelController {
private:
    CvPoint curLocation;
	Wheel * w_left;
	Wheel * w_right;
	Wheel * w_back;

#ifdef EMUATE_WHEEL_RESPONSE
	boost::thread_group threads;
	WheelEmulator * we_left;
	WheelEmulator * we_right;
	WheelEmulator * we_back;
#endif

public:
	WheelController(boost::asio::io_service &io) /*,
		w_left(io_service, "port1", 115200),
		w_right(io_service, "port2", 115200),
		w_back(io_service, "port3", 115200)*/
	{
		w_left = new Wheel(io, "COM10", 115200);
		w_right = new Wheel(io, "COM11", 115200);
		w_back = new Wheel(io, "COM12", 115200);

#ifdef EMUATE_WHEEL_RESPONSE
		we_left = new WheelEmulator(io, "COM14", 115200);
		we_right = new WheelEmulator(io, "COM15", 115200);
		we_back = new WheelEmulator(io, "COM16", 115200);
		threads.create_thread(boost::bind(&WheelEmulator::Run, we_left));
		threads.create_thread(boost::bind(&WheelEmulator::Run, we_right));
		threads.create_thread(boost::bind(&WheelEmulator::Run, we_back));

#endif
	};
	void Forward(int speed);
    void MoveTo(const CvPoint &);
    void Rotate(double degree);
    ~WheelController(){

#ifdef EMUATE_WHEEL_RESPONSE
		we_left->Stop();
		we_right->Stop();
		we_back->Stop();
		threads.join_all();
		delete we_left;
		delete we_right;
		delete we_back;

#endif
		delete w_left;
		delete w_right;
		delete w_back;

	};
};