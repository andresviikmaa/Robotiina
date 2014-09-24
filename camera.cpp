#include "camera.h"

void Camera::Camera(), VideoCap(0)
{

    if ( !cap.isOpened() )  // if not success, exit program
    {
       throw new Exception("Camera is missing");
    }

}
const Mat & Camera::Capture()
{
    bool bSuccess = cap.read(frame); // read a new frame from video

    if (!bSuccess) //if not success, break loop
    {
        throw new Exception("Cannot read a frame from video stream");
    }
    return frame;
}
const Mat &Camera::CaptureHSV() {
    cvtColor(frame, buffer, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
    return buffer;
}