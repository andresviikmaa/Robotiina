#include "robot.h"
#include "colorcalibrator.h"
#include "camera.h"
#include "stillcamera.h"
#include "wheelcontroller.h"
#include "objectfinder.h"

#include <opencv2/opencv.hpp>

std::pair<OBJECT, std::string> objects[] = {
	std::pair<OBJECT, std::string>(BALL, "Ball"),
	std::pair<OBJECT, std::string>(GATE, "Gate"),

};

std::map<OBJECT, std::string> OBJECT_LABELS(objects, objects + sizeof(objects) / sizeof(objects[0]));

Robot::Robot()
{
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
		objectThresholds[(OBJECT)i] = calibrator.GetObjectThresholds(i, OBJECT_LABELS[(OBJECT)i]);
    }
}

bool Robot::Launch(int argc, char* argv[])
{
	if (!ParseOptions(argc, argv)) return false;
	
	/*
	cv::namedWindow("Launch Robotiina", CV_WINDOW_AUTOSIZE);
	cv::createButton("button1", [](int state, void* self){ ((Robot*)self)->Configure(); }, this, CV_PUSH_BUTTON, 0);
	//cv::createButton("button2", Robot::ButtonConfigure, NULL, CV_PUSH_BUTTON, 0);
	*/

	camera = config.count("camera") ? new Camera(config["camera"].as<std::string>()) : new Camera(1);

	while (true) // mode loop
	{
		std::cout << "choose (C)onfigure or (R)un or <esc> to exit" << std::endl;
		cv::imshow("Launch Robotiina", cv::imread("ui/launch.png", CV_LOAD_IMAGE_COLOR));
		while (true) // key press loop
		{
			int key = cv::waitKey(30);
			if (key == 'c')
			{
				cv::destroyWindow("Launch Robotiina");
				CalibrateObjects();
				break;
			}
			else if (key == 'r')
			{
				cv::destroyWindow("Launch Robotiina");
				Run();
				break;
			}
			else if (key == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
			{
				std::cout << "mission completed" << std::endl;
				return true;
			}
		}
	}

	return true;

}

void Robot::Run()
{
    ObjectFinder finder(camera);
    WheelController wheels;
    while (state != END_OF_GAME)
    {
        if (LOCATE_BALL == state) {
			std::pair<int, double> location = finder.Locate(objectThresholds[BALL]);
			int HorizontalDev = location.first;
			float distance = location.second;

            //TODO: transform to real word coordinates
            if(location.second == -1) /* Ball not found */
            {
                wheels.Rotate(0.5 /* radians or degrees ?*/);
            }
            if (location.second != -1 && location.first != -1)
            {
                state = BALL_LOCATED;
            }

            /*wheels.MoveTo(location);*/

            //TODO: decide when to stop looking for balls
        }
        else if(BALL_LOCATED == state) {
            //TODO: start tribbler

			std::pair<int, double> location = finder.Locate(objectThresholds[BALL]);
			int HorizontalDev = location.first;
			float distance = location.second;

			if (distance < 100 && (HorizontalDev > -10 || HorizontalDev < 10)){
				//TODO: start catching the ball with tribbler
			}
			else if (distance < 100){
				//TODO: turn depending on HorizontalDev
				wheels.Rotate(HorizontalDev);
			}
			else{
				//TODO: move closer to ball
			}

            state = LOCATE_GATE;
        }
        else if (LOCATE_GATE == state)
        {
            /*CvPoint location = finder.Locate(objectThresholds[GATE]);*/
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
			break;
        }
    }
}

bool Robot::ParseOptions(int argc, char* argv[])
{
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("camera", po::value<std::string>(), "set camera index or path")
		;

	po::store(po::parse_command_line(argc, argv, desc), config);
	po::notify(config);

	if (config.count("help")) {
		std::cout << desc << std::endl;
		return false;
	}

	return true;
}
