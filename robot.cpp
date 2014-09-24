#include "robot.h"
#include "colorcalibrator.h"
#include <opencv2/opencv.hpp>

Robot::Robot()
{

    CalibrateObjects();
}

void Robot::CalibrateObjects()
{
    cv::Mat image = camera.Capture();
    ColorCalibrator calibrator;
    calibrator.LoacImage(image, NUMBER_OF_OBJECTS);
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
            const cv::Mat frame = camera.CaptureHSV();

            std::cout << "esc key is pressed by user" << std::endl;
            break;
        }
    }
        return 0;
}
