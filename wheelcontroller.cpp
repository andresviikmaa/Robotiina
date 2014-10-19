#include "wheelcontroller.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

WheelController::WheelController(boost::asio::io_service &io)
{
    using boost::property_tree::ptree;
    ptree pt;

    try {
    read_ini("conf/ports.ini", pt);
    }
    catch (...) {};

	try {
		w_left = new Wheel(io, pt.get<std::string>(std::to_string(ID_WHEEL_LEFT)), 115200);
	}
	catch (...) {
		w_left = new DummyWheel();
	}
	try {
		w_right = new Wheel(io, pt.get<std::string>(std::to_string(ID_WHEEL_RIGHT)), 115200);
	}
	catch (...) {
		w_right = new DummyWheel();
	}
	try {
		w_back = new Wheel(io, pt.get<std::string>(std::to_string(ID_WHEEL_BACK)), 115200);
	}
	catch (...) {
		w_back = new DummyWheel();
	}
};

void WheelController::Forward(int speed){

	w_left->Run(speed);
	w_right->Run(-speed);
	w_back->Run(0);

}
void WheelController::Rotate(bool direction, int speed){
	if (direction){
		w_left->Run(speed);
		w_right->Run(speed);
		w_back->Run(speed);
	}
	else {
		w_left->Run(-speed);
		w_right->Run(-speed);
		w_back->Run(-speed);
	}


}
void WheelController::Drive(int velocity, double direction){
	if (abs(velocity) > 190){
		if (velocity < 0){
			velocity = -190;
		}
		else{
			velocity = 190;
		}
	}

	w_left->Run((velocity*cos((150 - direction) * PI / 180.0)));
	w_right->Run((velocity*cos((30 - direction)  * PI / 180.0)));
	w_back->Run((velocity*cos((270 - direction)  * PI / 180.0)));

}

void WheelController::DriveRotate(int velocity, double direction, int rotate){
	if (abs(velocity) > 190){
		if (velocity > 0){
			velocity = 190;
		}
		else{
			velocity = -190;
		}
	}


	if ((abs(rotate) + velocity) > 190){
		if (rotate > 0){
			velocity = velocity - rotate;
		}
		else{
			velocity = velocity + rotate;
		}
			
	}


	w_left->Run((velocity*cos((150 - direction) * PI / 180.0)) + rotate);
	w_right->Run((velocity*cos((30 - direction)  * PI / 180.0)) + rotate);
	w_back->Run((velocity*cos((270 - direction)  * PI / 180.0)) + rotate);

	
	
}

bool WheelController::DriveToBall(double distance, double horizontalDev, double horizontalAngle, int desiredDistance){
	int speed;
	int rotate;
	//rotate calculation
	if (horizontalAngle > 200){
		rotate = (360 - horizontalAngle) * 2.5;
	}
	else{
		rotate = horizontalAngle * 2.5;
	}
	//if ball is close and center
	if (distance < desiredDistance && (horizontalDev > -10 && horizontalDev < 10)){
		//TODO: start catching the ball with tribbler
		Stop();
		return true;
	}
	//if ball is close but not center
	else if (distance < desiredDistance){
		//TODO: start tribbler
		if (horizontalDev < -10){
			Rotate(0, rotate);
		}
		else if (horizontalDev > 10){
			Rotate(1, rotate);
		}
		else{
			Stop();
		}
		return false;
		
	}
	//if ball is not close 
	else{
		//speed calculation
		if (distance > 700){
			speed = 150;
		}
		else{
			speed = distance * 0.35 - 91;
		}
		//driving commands
		if (horizontalDev > -20 && horizontalDev < 20){
			Drive(speed, horizontalAngle);
		}
		else if (horizontalDev >= 20){
			DriveRotate(speed, horizontalAngle, rotate);
		}
		else{
			DriveRotate(speed, horizontalAngle, -(rotate));
		}
		return false;
	}

}

void WheelController::Stop(){
	w_left->Stop();
	w_right->Stop();
	w_back->Stop();
}

bool WheelController::CheckStall(){
	if (w_left->stall || w_right->stall || w_back->stall){
		return true;
	}
	else{
		return false;
	}
}
