#pragma  once
#include "types.h"
#include "simpleserial.h"

class Wheel: public SimpleSerial
{
private:
	int speed = 0;
	int id = 0;
public:
	Wheel(boost::asio::io_service &io_service, std::string port = "port", unsigned int baud_rate = 115200) : SimpleSerial(io_service, port, baud_rate) {
		//TODO: Asking for ID, set id
		/*
		writeString("?\n");
		if (readLine() == "0"){
			std::ostringstream oss;
			oss << "id" << id << "\n";
			writeString(oss.str());
		}
		else{
			id = atoi(readLine().c_str());
		}	
		*/
	};
	void Run(int given_speed);
	void Stop();
	int Speed();
	
};