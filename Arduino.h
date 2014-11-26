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

class Arduino
{
protected:
	cv::Point3i sonars = {100,100,100};
public:
	Arduino(){}
	virtual ~Arduino(){}
	void Run(){};
	void AddButtonListener(ButtonClickListener *buttonListener){};
	const cv::Point3i &GetSonarReadings(){ return sonars; }
	virtual bool BallInTribbler(){ throw std::runtime_error("Not implemented"); };
	std::string GetDebugInfo();


};


class ArduinoBoard : public Arduino, SimpleSerial, ThreadedClass
{
private:
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::atomic<bool> ballInTribbler;

public:
	static Arduino Create();
	ArduinoBoard(boost::asio::io_service &io_service, std::string port = "", unsigned int baud_rate = 115200)  : ThreadedClass("Arduino"), SimpleSerial(io_service, port, baud_rate) {
		stop_thread = false;
		ballInTribbler = false;
		Start();
	};
	virtual ~ArduinoBoard(){
		WaitForStop();
	}
	void Run();

};











