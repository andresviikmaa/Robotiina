#include "wheelcontroller.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>


WheelController::WheelController(boost::asio::io_service &io, bool useDummyPorts)
{
    using boost::property_tree::ptree;
    ptree pt;
	if (useDummyPorts) {
		w_left = new SoftwareWheel();
		w_right = new SoftwareWheel();
		w_back = new SoftwareWheel();

	}
	else {
		read_ini("conf/ports.ini", pt);
		std::cout << "left wheel" << std::endl;
		w_left = new SerialWheel(io, pt.get<std::string>(std::to_string(ID_WHEEL_LEFT)), 115200);
		std::cout << "right wheel" << std::endl;
		w_right = new SerialWheel(io, pt.get<std::string>(std::to_string(ID_WHEEL_RIGHT)), 115200);
		std::cout << "back wheel" << std::endl;
		w_back = new SerialWheel(io, pt.get<std::string>(std::to_string(ID_WHEEL_BACK)), 115200);
		std::cout << "wheels done" << std::endl;

	}
};

void WheelController::Forward(int speed){

	DriveRotate(speed * 1.1547, 0, 0);

}
cv::Point3d WheelController::Rotate(bool direction, double speed){
	return DriveRotate(0,0, direction ? speed : -speed);
}
cv::Point3d WheelController::Drive(double velocity, double direction){
	return DriveRotate(velocity, direction, 0);
}

cv::Point3d WheelController::DriveRotate(double velocity, double direction, double rotate){
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

	lastSpeed.x = sin(direction* PI / 180.0)* velocity + rotate;
	lastSpeed.y = cos(direction* PI / 180.0)* velocity + rotate,
	lastSpeed.z = rotate;

	w_left->SetSpeed((velocity*cos((150 - direction) * PI / 180.0)) + rotate);
	w_right->SetSpeed((velocity*cos((30 - direction)  * PI / 180.0)) + rotate);
	w_back->SetSpeed((velocity*cos((270 - direction)  * PI / 180.0)) + rotate);

	return lastSpeed;
	
}

cv::Point3d WheelController::Stop(){
	return Drive(0,0);
}

bool WheelController::IsStalled(){
	return w_left->IsStalled() || w_right->IsStalled() || w_back->IsStalled();
}
