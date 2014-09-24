#include "robot.h"
#include "colorcalibrator.h"
#include "camera.h"
#include "stillcamera.h"
#include "wheelcontroller.h"
#include "objectfinder.h"

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
        objectThresholds[(OBJECT)i] = calibrator.GetObjectThresholds(i);
    }
}

int Robot::run()
{
    ObjectFinder finder(camera);
    WheelController wheels;
    while (state != END_OF_GAME)
    {
        if (LOCATE_BALL == state) {
            CvPoint location = finder.Locate(objectThresholds[BALL]);
            //TODO: transform to real word coordinates
            if(location.x == -1) /* Ball not found */
            {
                wheels.Rotate(0.5 /* radians or degrees ?*/);
            }
            if (location.x == 0 && location.y == 0)
            {
                state = BALL_LOCATED;
            }
            wheels.MoveTo(location);
            //TODO: decide when to stop looking for balls
        }
        else if(BALL_LOCATED == state) {
            //TODO: start tribbler
            state = LOCATE_GATE;
        }
        else if (LOCATE_GATE == state)
        {
            CvPoint location = finder.Locate(objectThresholds[GATE]);
            //TODO: how
            wheels.Rotate(0);
            state = GATE_LOCATED;
        }
        else if(GATE_LOCATED == state)
        {
            //TODO: kick ball
            state = LOCATE_BALL;
        }

        if (cv::waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
          //  const cv::Mat frame = camera->Capture();

            std::cout << "esc key is pressed by user" << std::endl;
            state = END_OF_GAME;
        }
    }
        return 0;
}
