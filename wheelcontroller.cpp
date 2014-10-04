#include "wheelcontroller.h"

void WheelController::Forward(int speed){

	w_left->Run(speed);
	w_right->Run(-speed);
	w_back->Run(0);

}
void WheelController::Rotate(bool direction){
	if (direction){
		w_left->Run(50);
		w_right->Run(50);
		w_back->Run(50);
	}
	else {
		w_left->Run(-50);
		w_right->Run(-50);
		w_back->Run(-50);
	}


}
void WheelController::Drive(int velocity, double direction){
	
	w_left->Run(velocity*cos((150 - direction) * PI / 180.0));
	w_right->Run(velocity*cos((30 - direction)  * PI / 180.0));
	w_back->Run(velocity*cos((270 - direction)  * PI / 180.0));

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
	w_left->Run((velocity*cos((150 - direction) * PI / 180.0)) + rotate);
	w_right->Run((velocity*cos((30 - direction)  * PI / 180.0)) + rotate);
	w_back->Run((velocity*cos((270 - direction)  * PI / 180.0)) + rotate);

}