#include "robot.h"
#include "colorcalibrator.h"
#include "camera.h"
#include "stillcamera.h"

#include <opencv2/opencv.hpp>
#define USE_STILL_CAMERA
Robot::Robot()
{
    #ifdef USE_STILL_CAMERA
    camera = new StillCamera("test_images/face.jpg");
    #else
    camera = new Camera();
    #endif

    CalibrateObjects();
}
Robot::~Robot()
{
    delete camera;
}
void Robot::CalibrateObjects()
{
    cv::Mat image = camera->Capture();
    ColorCalibrator calibrator;
    calibrator.LoadImage(image, NUMBER_OF_OBJECTS);
    for(int i = 0; i < NUMBER_OF_OBJECTS; i++)
    {
        objectThresholds[(OBJECTS)i] = calibrator.GetObjectThresholds(i);
    }
}

int Robot::run()
{
    while (true)
    {
        if (cv::waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            const cv::Mat frame = camera->Capture();

            std::cout << "esc key is pressed by user" << std::endl;
            break;
        }
    }
        return 0;
}
