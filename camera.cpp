#include "camera.h"
#include <opencv2/opencv.hpp>

Camera::Camera(): cv::VideoCapture(0)
{

    if ( !isOpened() )  // if not success, exit program
    {
       throw std::string("Camera is missing");
    }

}
const cv::Mat &Camera::Capture()
{
    bool bSuccess = read(frame); // read a new frame from video

    if (!bSuccess) //if not success, break loop
    {
        throw std::string("Cannot read a frame from video stream");
    }
    return frame;
}
const cv::Mat &Camera::CaptureHSV() {
    cvtColor(frame, buffer, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
    return buffer;
}