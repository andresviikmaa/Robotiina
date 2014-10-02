#include "wheelcontroller.h"

void WheelController::Forward(int speed){
	w_left.Run(speed);
	w_right.Run(speed);
	w_back.Run(speed);
}
void WheelController::Rotate(double degree){
	w_left.Run(10);
	w_right.Run(20);
	w_back.Run(10);
}