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
#include <boost/algorithm/string.hpp>

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

Robot::Robot(boost::asio::io_service &io) : io(io)
{
	camera = NULL;
    state = STATE_NONE;
    wheels = new WheelController(io);
}
Robot::~Robot()
{
	if (camera)
		delete camera;
    if(wheels)
        delete wheels;

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
		if (STATE_CRASH == state){
			wheels->Stop();
		}
        if (STATE_LOCATE_BALL == state) {
			cv::Point3d location = finder.Locate(objectThresholds[BALL], camera->Capture());
			double distance = location.x;
			double HorizontalDev = location.y;
			double HorizontalAngle = location.z;
          
			if (location.x == -1 && location.y == -1 && location.z == -1) /* Ball not found */
            {
                wheels->Rotate(1);
            }
			if (location.x != -1 && location.y != -1) /*Ball found*/
            {
				wheels->Stop();
                state = STATE_BALL_LOCATED;
            }
            
        }
        if(STATE_BALL_LOCATED == state) {
			cv::Point3d location = finder.Locate(objectThresholds[BALL], camera->Capture());
			double distance = location.x;
			double HorizontalDev = location.y;
			double HorizontalAngle = location.z;

			//If ball is lost
			if (distance == -1 && HorizontalDev == -1 && HorizontalAngle == -1){ 
				state = STATE_LOCATE_BALL;
			}			
			else if (distance < 300 && (HorizontalDev > -50 && HorizontalDev < 50)){
				//TODO: start catching the ball with tribbler
				wheels->Stop();
			}
			else if (distance < 300){
				//TODO: start tribbler
				//TODO: turn depending on HorizontalDev
				wheels->Stop();
			}
			else{
				int speed = distance * 0.05 - 5;
				if (HorizontalDev > -50 && HorizontalDev < 50){
					wheels->Drive(speed, HorizontalAngle);
				}
				else if (HorizontalDev >= 50){
					wheels->DriveRotate(speed, HorizontalAngle, 15);
				}
				else{
					wheels->DriveRotate(speed, HorizontalAngle, -15);
				}			
				
			}
			std::cout << "distance2 " << HorizontalDev << '\n';
			
            //state = STATE_LOCATE_GATE;
        }
        if (STATE_LOCATE_GATE == state)
        {
            /*CvPoint location = finder.Locate(objectThresholds[GATE]);*/
            //TODO: how
            wheels->Rotate(0);
            state = STATE_GATE_LOCATED;
        }
        if(STATE_GATE_LOCATED == state)
        {
            //TODO: kick ball
            state = STATE_LOCATE_BALL;
        }
        if(STATE_REMOTE_CONTROL == state) {

        }
		if (wheels->CheckStall()){
			state = STATE_CRASH;
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
std::string Robot::ExecuteRemoteCommand(const std::string &command){
    std::stringstream response;
    boost::mutex::scoped_lock lock(remote_mutex); //allow one command at a time
    std::vector<std::string> tokens;
    boost::split(tokens, command, boost::is_any_of(";"));
    std::string query = tokens[0];
    STATE s = (STATE)GetState();
    if(query == "status") response << s;
    else if(query == "remote") SetState(STATE_REMOTE_CONTROL);
    else if(query == "cont") SetState(STATE_LOCATE_BALL);
    else if(query == "reset") SetState(STATE_NONE);
    else if(query == "exit") SetState(STATE_END_OF_GAME);
    else if (STATE_REMOTE_CONTROL == s) {
        if (query == "drive" && tokens.size() == 3) {
            int speed = atoi(tokens[1].c_str());
            float direction = atof(tokens[2].c_str());
            wheels->Drive(speed, direction);
        }
    }
    return response.str();
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
