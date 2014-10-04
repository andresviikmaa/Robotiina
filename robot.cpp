#include "robot.h"
#include "colorcalibrator.h"
#include "autocalibrator.h"
#include "camera.h"
#include "stillcamera.h"
#include "wheelcontroller.h"
#include "objectfinder.h"
#include "dialog.h"
#include "wheel.h"

#include <opencv2/opencv.hpp>

#define BUTTON(dialog, name, new_state) \
dialog.createButton(name, [](int state, void* self){ ((Robot*)self)->state = new_state; }, this, CV_PUSH_BUTTON, 0);

std::pair<OBJECT, std::string> objects[] = {
	std::pair<OBJECT, std::string>(BALL, "Ball"),
    std::pair<OBJECT, std::string>(GATE, "Gate"),
    std::pair<OBJECT, std::string>(FIELD, "Field"),
    std::pair<OBJECT, std::string>(INNER_BORDER, "Inner Border2"),
    std::pair<OBJECT, std::string>(OUTER_BORDER, "Outer Border2"),

};

std::map<OBJECT, std::string> OBJECT_LABELS(objects, objects + sizeof(objects) / sizeof(objects[0]));

Robot::Robot() : io()
{
	camera = NULL;
}
Robot::~Robot()
{
	if (camera)
		delete camera;
}
void Robot::CalibrateObjects(bool autoCalibrate/* = false*/)
{
    cv::Mat image = camera->Capture();
    ColorCalibrator* calibrator = autoCalibrate ? new AutoCalibrator() : new ColorCalibrator();
    calibrator->LoadImage(image);

    for(int i = 0; i < NUMBER_OF_OBJECTS; i++) {
        objectThresholds[(OBJECT) i] = calibrator->GetObjectThresholds(i, OBJECT_LABELS[(OBJECT) i]);
    }
	
    delete calibrator;

}

bool Robot::Launch(int argc, char* argv[])
{
	if (!ParseOptions(argc, argv)) return false;

	
	if (config.count("camera"))
		camera = new StillCamera(config["camera"].as<std::string>());
	else
        camera = new Camera(0);

    Run();
}

void Robot::Run()
{
    ObjectFinder finder;
	WheelController wheels(io);

    while (state != STATE_END_OF_GAME)
    {
        if (STATE_NONE == state) {

            Dialog launchWindow("Launch Robotiina", CV_WINDOW_AUTOSIZE);
            BUTTON(launchWindow, "Configure USB devices", STATE_CONFIGURE_USB)
            BUTTON(launchWindow, "AutoCalibrate objects", STATE_AUTOCALIBRATE)
            BUTTON(launchWindow, "ManualCalibrate objects", STATE_CALIBRATE)
			BUTTON(launchWindow, "Start Robot", STATE_LAUNCH)
            BUTTON(launchWindow, "Exit", STATE_END_OF_GAME)
            launchWindow.show();

        }
        if (STATE_CALIBRATE == state) {
            CalibrateObjects();
            state = STATE_NONE;
        }
        if (STATE_AUTOCALIBRATE == state) {
            CalibrateObjects(true);
            state = STATE_NONE;
        }
		if (STATE_LAUNCH == state) {
			try {
				CalibrationConfReader calibrator;
				for (int i = 0; i < NUMBER_OF_OBJECTS; i++) {
					objectThresholds[(OBJECT)i] = calibrator.GetObjectThresholds(i, OBJECT_LABELS[(OBJECT)i]);
				}
				state = STATE_LOCATE_BALL;
			}
			catch (...){
				//TODO: display error
				state = STATE_NONE; // no conf
			}
		}
        if (STATE_LOCATE_BALL == state) {
			std::pair<int, double> location = finder.Locate(objectThresholds[BALL], camera->Capture());
			int HorizontalDev = location.first;
			float distance = location.second;

            //TODO: transform to real word coordinates
            if(location.second == -1) /* Ball not found */
            {
                //wheels.Rotate(0.5 /* radians or degrees ?*/);
            }
            if (location.second != -1 && location.first != -1)
            {
                state = STATE_BALL_LOCATED;
            }

            wheels.DriveRotate(190,90, 50);

            //TODO: decide when to stop looking for balls
        }
        if(STATE_BALL_LOCATED == state) {
            //TODO: start tribbler

			std::pair<int, double> location = finder.Locate(objectThresholds[BALL], camera->Capture());
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

            state = STATE_LOCATE_GATE;
        }
        if (STATE_LOCATE_GATE == state)
        {
            /*CvPoint location = finder.Locate(objectThresholds[GATE]);*/
            //TODO: how
            wheels.Rotate(0);
            state = STATE_GATE_LOCATED;
        }
        if(STATE_GATE_LOCATED == state)
        {
            //TODO: kick ball
            state = STATE_LOCATE_BALL;
        }

        if (cv::waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
          //  const cv::Mat frame = camera->Capture();

            std::cout << "esc key is pressed by user" << std::endl;
            //state = STATE_END_OF_GAME;
            state = STATE_NONE;
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
