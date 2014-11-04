#include "wheelcontroller.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#define deg150 (150.0 * PI / 180.0)
#define deg30 (30.0 * PI / 180.0)
#define deg270 (270.0 * PI / 180.0)

WheelController::WheelController()
{
	w_left = NULL;
	w_right = NULL;
	w_back = NULL;
};

void WheelController::InitWheels(boost::asio::io_service &io, bool useDummyPorts/* = false*/)
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
	w_left->Start();
	w_right->Start();
	w_back->Start();

}

void WheelController::StopWheels()
{
	if (w_left != NULL) {
		w_left->Stop();
		delete w_left;
		w_left = NULL;
	}
	if (w_right != NULL) {
		w_right->Stop();
		delete w_right;
		w_right = NULL;
	}
	if (w_back != NULL) {
		w_back->Stop();
		delete w_back;
		w_back = NULL;

	}
};
WheelController::~WheelController()
{
	StopWheels();
}
void WheelController::Forward(int speed) {

	DriveRotate(speed * 1.1547, 0, 0);

}
cv::Point3d WheelController::Rotate(bool direction, double speed)
{
	return DriveRotate(0,0, direction ? speed : -speed);
}
cv::Point3d WheelController::Drive(double velocity, double direction)
{
	return DriveRotate(velocity, direction, 0);
}

cv::Point3d WheelController::DriveRotate(double velocity, double direction, double rotate)
{
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

	lastSpeed.x = velocity; // sin(direction* PI / 180.0)* velocity + rotate;
	lastSpeed.y = direction; //cos(direction* PI / 180.0)* velocity + rotate,
	lastSpeed.z = rotate;

	auto speeds = CalculateWheelSpeeds(velocity, direction, rotate);
	w_left->SetSpeed(speeds.x);
	w_right->SetSpeed(speeds.y);
	w_back->SetSpeed(speeds.z);

	return lastSpeed;
	
}
cv::Point3d WheelController::CalculateWheelSpeeds(double velocity, double direction, double rotate)
{
	return cv::Point3d(
		(velocity*cos((150 - direction) * PI / 180.0)) + rotate,
		(velocity*cos((30 - direction)  * PI / 180.0)) + rotate,
		(velocity*cos((270 - direction)  * PI / 180.0)) + rotate
	);
}
cv::Point3d WheelController::Stop()
{
	return Drive(0,0);
}

bool WheelController::IsStalled()
{
	return false;
	return w_left->IsStalled() || w_right->IsStalled() || w_back->IsStalled();
}
bool WheelController::HasError()
{
	return w_left->HasError() || w_right->HasError() || w_back->HasError();
}

cv::Point3d WheelController::GetWheelSpeeds()
{
	return cv::Point3d(w_left->GetSpeed(), w_right->GetSpeed(), w_back->GetSpeed());
}

void WheelController::GetRobotSpeed(double &velocity, double &direction, double &rotate)
{
	cv::Point3d speeds = GetWheelSpeeds();
	velocity = direction = rotate = 0;
	double a, b, c, u, v, w;
	/*
	a = x *[cos(u) * cos(y) + sin(u) * sin(y)] + z
	b = x *[cos(v) * cos(y) + sin(v) * sin(y)] + z
	c = x *[cos(w) * cos(y) + sin(w) * sin(y)] + z
	*/
	if (abs(speeds.z - speeds.x) > 0.0000001) { // c - a == 0
		a = speeds.x; b = speeds.y; c = speeds.z;
		u = deg150; v = deg30; w = deg270;
	}
	else if (abs(speeds.x - speeds.y) > 0.0000001) {
		a = speeds.y; b = speeds.z; c = speeds.x;
		u = deg30; v = deg270; w = deg30;
	}
	else if (abs(speeds.z - speeds.y) > 0.0000001) {
		a = speeds.z; b = speeds.x; c = speeds.y;
		u = deg270; v = deg30; w = deg150;
	}
	else {
		// all equal, rotation only
		rotate = speeds.x;
		return;

	}
	double s = (b - a) / (c - a);
	double directionInRad = atan(((cos(v) - cos(u)) - s * (cos(w) - cos(u))) / (s * (sin(w) - sin(u)) - (sin(v) - sin(u))));
	if (directionInRad < 0) directionInRad += 2 * PI;
	direction = directionInRad / PI * 180;
	velocity = (a - c) / ((cos(u) - cos(w)) * cos(directionInRad) + (sin(u) - sin(w)) * sin(directionInRad));
	rotate = c - (velocity * cos(w - directionInRad));

}

void WheelController::GetTargetSpeed(double &velocity, double &direction, double &rotate) 
{
	velocity = lastSpeed.x;
	direction = lastSpeed.y;
	rotate = lastSpeed.z;
}
