#pragma once
#include "types.h"
#include "ThreadedClass.h"
#include "simpleserial.h"
#include <boost/atomic.hpp>

class ButtonClickListener
{
	virtual void OnStartButtonClicked() = 0;
	virtual void OnGateButtonClicked() = 0;
};

class Audrino : public ThreadedClass
{
protected:
	cv::Point3i sonars;
public:
	Audrino(){}
	virtual ~Audrino(){}
	void Run(){};
	void AddButtonListener(ButtonClickListener *buttonListener){};
	const cv::Point3i &GetSonarReadings(){ return sonars; }
	virtual bool BallInTribbler(){ throw std::runtime_error("Not implemented"); };
	std::string GetDebugInfo();

};


class AudrinoBoard : public Audrino, SimpleSerial
{
private:
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::atomic<bool> ballInTribbler;

public:
	AudrinoBoard(boost::asio::io_service &io_service, std::string port = "", unsigned int baud_rate = 115200) : SimpleSerial(io_service, port, baud_rate) {
		stop_thread = false;
		ballInTribbler = false;
	};
	virtual ~AudrinoBoard(){
		writeString("d\n");//discharge
	}
	void Run();

};











