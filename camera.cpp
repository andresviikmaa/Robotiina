#include "camera.h"
#include <opencv2/opencv.hpp>

Camera::Camera(const std::string &device)
{

	cap = new cv::VideoCapture(device.c_str());
	if (!cap->isOpened())  // if not success, exit program
    {
		throw std::runtime_error("Camera not found");
    }
	//cap->set(CV_CAP_PROP_EXPOSURE, 0);

}
Camera::Camera(int device)
{
	
	cap = new cv::VideoCapture(device);
	if (!cap->isOpened())  // if not success, exit program
	{
		throw std::runtime_error("Camera is missing");
	}
	//cap->set(CV_CAP_PROP_EXPOSURE,0);

}

const cv::Mat &Camera::Capture()
{
	if (cap->isOpened())
		
		*cap >> frame;
		

    return frame;
}
const cv::Mat &Camera::CaptureHSV() {
    cvtColor(frame, buffer, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
    return buffer;
}
