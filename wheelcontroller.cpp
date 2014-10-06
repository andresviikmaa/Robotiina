#include "wheelcontroller.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

WheelController::WheelController(boost::asio::io_service &io)
{
    using boost::property_tree::ptree;
    ptree pt;
    try {
        read_ini("conf/wheels.ini", pt);
    } catch(...) {
        pt.put("left.port", "COM6");
        pt.put("right.port", "COM8");
        pt.put("back.port", "COM10");
        write_ini("conf/wheels.ini", pt);
    }


    w_left = new Wheel(io, pt.get<std::string>("left.port"), 115200);
    w_right = new Wheel(io, pt.get<std::string>("right.port"), 115200);
    w_back = new Wheel(io, pt.get<std::string>("back.port"), 115200);
};

void WheelController::Forward(int speed){

	is_stall[0] = w_left->Run(speed);
	is_stall[1] = w_right->Run(-speed);
	is_stall[2] = w_back->Run(0);

}
void WheelController::Rotate(bool direction){
	if (direction){
		is_stall[0] = w_left->Run(50);
		is_stall[1] = w_right->Run(50);
		is_stall[2] = w_back->Run(50);
	}
	else {
		is_stall[0] = w_left->Run(-50);
		is_stall[1] = w_right->Run(-50);
		is_stall[2] = w_back->Run(-50);
	}


}
void WheelController::Drive(int velocity, double direction){
	
	is_stall[0] = w_left->Run(velocity*cos((150 - direction) * PI / 180.0));
	is_stall[1] = w_right->Run(velocity*cos((30 - direction)  * PI / 180.0));
	is_stall[2] = w_back->Run(velocity*cos((270 - direction)  * PI / 180.0));

}

void WheelController::DriveRotate(int velocity, double direction, int rotate){
	if (abs(velocity) > 190 || abs(direction) > 190){
		std::cout << "Invalid arguments: too big";
		return;
	}
	else if ((velocity + rotate) > 190){
		velocity = 190 - rotate;
	}
	else if ((velocity + rotate) < -190){
		velocity = -190 - rotate;
	}
	
	is_stall[0] = w_left->Run((velocity*cos((150 - direction) * PI / 180.0)) + rotate);
	is_stall[1] = w_right->Run((velocity*cos((30 - direction)  * PI / 180.0)) + rotate);
	is_stall[2] = w_back->Run((velocity*cos((270 - direction)  * PI / 180.0)) + rotate);

	
	
}

void WheelController::Stop(){
	w_left->Stop();
	w_right->Stop();
	w_back->Stop();
}

bool WheelController::CheckStall(){
	if (is_stall[0] != "stall:0" & is_stall[1] != "stall:0" & is_stall[2] != "stall:0"){
		return true;
	}
	else{
		return false;
	}
}