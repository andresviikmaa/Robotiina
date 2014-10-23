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
//		throw;
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

	cv::namedWindow("wheels");
	cv::Point2i c(200, 200);
	cv::Mat infoWindow(c.x * 2, c.y * 2, CV_8UC3, cv::Scalar::all(0));
	/*
	pygame.draw.line(self.screen, self.WHITE, (305, 314), (
		305 + np.sin(self.logic.motors.direction)*self.logic.motors.speed,
		314 - np.cos(self.logic.motors.direction)*self.logic.motors.speed), 1)
	*/
	cv::Point2i e(sin(direction* PI / 180.0)* velocity + rotate, cos(direction* PI / 180.0)*velocity + rotate);
	cv::line(infoWindow, c, c + e, cv::Scalar(255, 255, 255), 1, 8, 0);
	cv::imshow("wheels", infoWindow);

	w_left->Run((velocity*cos((150 - direction) * PI / 180.0)) + rotate);
	w_right->Run((velocity*cos((30 - direction)  * PI / 180.0)) + rotate);
	w_back->Run((velocity*cos((270 - direction)  * PI / 180.0)) + rotate);

	return cv::Point3f(e.x, e.y, 0);
	
}

cv::Point3f WheelController::DriveToBall(double distance, double horizontalDev, double horizontalAngle, int desiredDistance){
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
	w_left->Stop();
	w_right->Stop();
	w_back->Stop();
	return cv::Point3f(0, 0, 0);
}

bool WheelController::CheckStall(){
	if (w_left->stall || w_right->stall || w_back->stall){
		return true;
	}
	else{
		return false;
	}
}
