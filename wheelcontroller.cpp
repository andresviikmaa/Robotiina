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
cv::Point3f WheelController::Rotate(bool direction, int speed){
	return DriveRotate(0,0, direction ? speed : -speed);
}
cv::Point3f WheelController::Drive(int velocity, double direction){
	return DriveRotate(velocity, direction, 0);
}

cv::Point3f WheelController::DriveRotate(int velocity, double direction, int rotate){
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

cv::Point3f WheelController::DriveToBall(double distance, double horizontalDev, double horizontalAngle, int desiredDistance, DummyCoilBoard *coilBoard){
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
		/* if (coilBoard->BallInTribbler()){
			return Stop();
		}
		else{
			Drive(15, horizontalAngle);
		}
		*/
		return Stop();
	}
	//if ball is close but not center
	else if (distance < desiredDistance){
		//TODO: start tribbler
		if (horizontalDev < -10){
			return Rotate(0, rotate);
		}
		else if (horizontalDev > 10){
			return Rotate(1, rotate);
		}
		else{
			return Stop();
		}
	}
	//if ball is not close 
	else{
		//speed calculation
		if (distance > 700){
			speed = 150;
		}
		else{
			speed = distance * 0.26 - 32;
		}
		//driving commands
		if (horizontalDev > -20 && horizontalDev < 20){
			return Drive(speed, horizontalAngle);
		}
		else if (horizontalDev >= 20){
			return DriveRotate(speed, horizontalAngle, rotate);
		}
		else{
			return DriveRotate(speed, horizontalAngle, -(rotate));
		}
		
	}

}

cv::Point3f WheelController::Stop(){
	return Drive(0,0);
}

bool WheelController::CheckStall(){
	if (w_left->IsStalled() || w_right->IsStalled() || w_back->IsStalled()){
		return true;
	}
	else{
		return false;
	}
}
