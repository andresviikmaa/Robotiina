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
		std::cout << "error opening left wheel " << std::endl;
		w_left = new DummyWheel();
		throw;
	}
	try {
		w_right = new Wheel(io, pt.get<std::string>(std::to_string(ID_WHEEL_RIGHT)), 115200);
	}
	catch (...) {
		std::cout << "error opening right wheel " << std::endl;
		w_right = new DummyWheel();
	}
	try {
		w_back = new Wheel(io, pt.get<std::string>(std::to_string(ID_WHEEL_BACK)), 115200);
	}
	catch (...) {
		std::cout << "error opening back wheel " << std::endl;
		w_back = new DummyWheel();
	}
};

void WheelController::Forward(int speed){

	w_left->Run(speed);
	w_right->Run(-speed);
	w_back->Run(0);

}
void WheelController::Rotate(bool direction, int speed){

	DriveRotate(0,0, direction ? speed : -speed);
}
void WheelController::Drive(int velocity, double direction){
	DriveRotate(velocity, direction, 0);

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

	cv::namedWindow("wheels");
	cv::Point2i c(200, 200);
	cv::Mat infoWindow(c.x * 2, c.y * 2, CV_8UC3, cv::Scalar::all(0));
	/*
	pygame.draw.line(self.screen, self.WHITE, (305, 314), (
		305 + np.sin(self.logic.motors.direction)*self.logic.motors.speed,
		314 - np.cos(self.logic.motors.direction)*self.logic.motors.speed), 1)
	*/
	cv::Point2i e(sin(direction* PI / 180.0)* velocity, cos(direction* PI / 180.0)*velocity);
	cv::line(infoWindow, c, c + e, cv::Scalar(255, 255, 255), 1, 8, 0);
	cv::imshow("wheels", infoWindow);

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
