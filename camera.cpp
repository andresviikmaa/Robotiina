#include "camera.h"
#include <opencv2/opencv.hpp>

Camera::Camera(const std::string &device)
{

	cap = new cv::VideoCapture(device.c_str());
	if (!cap->isOpened())  // if not success, exit program
    {
       throw std::string("Camera not found");
    }

}
Camera::Camera(int device)
{

	cap = new cv::VideoCapture(device);
	if (!cap->isOpened())  // if not success, exit program
	{
		throw std::string("Camera is missing");
	}

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