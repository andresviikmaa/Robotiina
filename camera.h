#pragma  once
#include <opencv2/opencv.hpp>

class Camera: protected cv::VideoCapture
{
private:
    cv::Mat frame, buffer;
public:
    Camera();
    const cv::Mat & Capture();
    const cv::Mat & CaptureHSV();
    ~Camera(){ }

};