#include "robot.h"
#include "colorcalibrator.h"
#include "autocalibrator.h"
#include "camera.h"
#include "stillcamera.h"
#include "wheelcontroller.h"
#include "objectfinder.h"
#include "mousefinder.h"
#include "dialog.h"
#include "wheel.h"
#include "ComPortScanner.h"

#include <opencv2/opencv.hpp>
#include <boost/algorithm/string.hpp>
#include <chrono>
#include <thread>

#define STATE_BUTTON(dialog, name, new_state) \
dialog.createButton(name, [](int state, void* self){ ((Robot*)self)->state = new_state; }, this, CV_PUSH_BUTTON, 0);
#define BUTTON(dialog, name, function_body) \
dialog.createButton(name, [](int state, void* self){ function_body }, this, CV_PUSH_BUTTON, 0);

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
    //wheels = new WheelController(io);
}
Robot::~Robot()
{
	if (camera)
		delete camera;
    if(wheels)
        delete wheels;
	if (finder)
		delete finder;

}
void Robot::CalibrateObjects(const cv::Mat &image, bool autoCalibrate/* = false*/)
{
    ColorCalibrator* calibrator = autoCalibrate ? new AutoCalibrator() : new ColorCalibrator();
	cv::Mat image2 = camera->Capture();
//return;
    calibrator->LoadImage(image2);

    for(int i = 0; i < NUMBER_OF_OBJECTS; i++) {
        objectThresholds[(OBJECT) i] = calibrator->GetObjectThresholds(i, OBJECT_LABELS[(OBJECT) i]);
    }
	
    delete calibrator;

}

bool Robot::Launch(int argc, char* argv[])
{
	if (!ParseOptions(argc, argv)) return false;

	std::cout << "Initializing camera... " << std::endl;
	if (config.count("camera"))
		camera = new StillCamera(config["camera"].as<std::string>());
	else
		camera = new Camera(0);
	std::cout << "Done" << std::endl;

	if (config.count("locate_cursor"))
		finder = new MouseFinder();
	else
		finder = new ObjectFinder();

	std::cout << "Checking COM ports... " << std::endl;
	{ // new scope for scanner variable
		ComPortScanner scanner;
		if (!scanner.Verify(io)){
			std::cout << "Chek failed, rescanning all ports" << std::endl;
			scanner.Scan(io);
		}
	}
	std::cout << "Done" << std::endl;

	std::cout << "Starting Robot" << std::endl;
    Run();
}

void Robot::Run()
{
 
    while (state != STATE_END_OF_GAME)
    {
//		cv::Mat image = camera->Capture();

        if (STATE_NONE == state) {

            Dialog launchWindow("Launch Robotiina", CV_WINDOW_AUTOSIZE);
			STATE_BUTTON(launchWindow, "AutoCalibrate objects", STATE_AUTOCALIBRATE)
			STATE_BUTTON(launchWindow, "ManualCalibrate objects", STATE_CALIBRATE)
			STATE_BUTTON(launchWindow, "Start Robot", STATE_LAUNCH)
			STATE_BUTTON(launchWindow, "Remote Control", STATE_REMOTE_CONTROL)
			STATE_BUTTON(launchWindow, "Manual Control", STATE_MANUAL_CONTROL)
			STATE_BUTTON(launchWindow, "Exit", STATE_END_OF_GAME)
            launchWindow.show();

        }
        if (STATE_CALIBRATE == state) {
			cv::Mat image;// = camera->Capture();
			CalibrateObjects(image);
            state = STATE_NONE;
        }
        if (STATE_AUTOCALIBRATE == state) {
			cv::Mat image = camera->Capture();
            CalibrateObjects(image, true);
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
			//Backwards
			wheels->Drive(50, 180);
			std::chrono::milliseconds dura(1000);
			std::this_thread::sleep_for(dura);
			wheels->Stop();
			//Turn a littlebit
			wheels->Rotate(1, 100);
			std::this_thread::sleep_for(dura);
			wheels->Stop();
			//Check again
			if (!wheels->CheckStall()){
				state = STATE_LOCATE_BALL;
			}            
		}
        if (STATE_LOCATE_BALL == state) {
			cv::Point3d location = finder->Locate(objectThresholds[BALL], camera->Capture());
			double distance = location.x;
			double HorizontalDev = location.y;
			double HorizontalAngle = location.z;
			if (location.x == -1 && location.y == -1 && location.z == -1) /* Ball not found */
            {
                wheels->Rotate(1, 10);
            }
			if (location.x != -1 && location.y != -1) /*Ball found*/
            {
				wheels->Stop();
                state = STATE_BALL_LOCATED;
            }
            
        }
        if(STATE_BALL_LOCATED == state) {
			cv::Point3d location = finder->Locate(objectThresholds[BALL], camera->Capture());
			double distance = location.x;
			double HorizontalDev = location.y;
			double HorizontalAngle = location.z;

			//If ball is lost
			if (distance == -1 && HorizontalDev == -1 && HorizontalAngle == -1){ 
				state = STATE_LOCATE_BALL;
			}			
			else if (distance < 250 && (HorizontalDev > -50 && HorizontalDev < 50)){
				//TODO: start catching the ball with tribbler
				wheels->Stop();
			}
			else if (distance < 250){
				//TODO: start tribbler
				//TODO: turn depending on HorizontalDev
				wheels->Stop();
			}
			else{
				if (distance > 700){
					speed = 150;
				}
				else{
					speed = distance * 0.35 - 91;
				}
                                speed = 50;
					
				if (HorizontalDev > -50 && HorizontalDev < 50){
					wheels->Drive(speed, HorizontalAngle);
				}
				else if (HorizontalDev >= 50){
					wheels->DriveRotate(speed, HorizontalAngle, 0);
				}
				else{
					wheels->DriveRotate(speed, HorizontalAngle, 0);
				}
			}
            //state = STATE_LOCATE_GATE;
        }
        if (STATE_LOCATE_GATE == state)
        {
            /*CvPoint location = finder.Locate(objectThresholds[GATE]);*/
            //TODO: how
            wheels->Rotate(1,10);
            state = STATE_GATE_LOCATED;
        }
        if(STATE_GATE_LOCATED == state)
        {
            //TODO: kick ball
            state = STATE_LOCATE_BALL;
        }
        if(STATE_REMOTE_CONTROL == state) {
			Dialog launchWindow("Remote Control Mode Enabed", CV_WINDOW_AUTOSIZE);
			STATE_BUTTON(launchWindow, "Back", STATE_NONE)
			launchWindow.show();

        }

		if (STATE_MANUAL_CONTROL == state) {
			Dialog manualWindow("Manual Control Mode Enabed", CV_WINDOW_AUTOSIZE);
			BUTTON(manualWindow, "Move Left", ((Robot*)self)->wheels->Drive(20, 90);)
				BUTTON(manualWindow, "Move Right", ((Robot*)self)->wheels->Drive(20, 270);)
				BUTTON(manualWindow, "Move Forward", ((Robot*)self)->wheels->Drive(20, 0);)
				BUTTON(manualWindow, "Move Back", ((Robot*)self)->wheels->Drive(-20, 0);)
				BUTTON(manualWindow, "Rotate", ((Robot*)self)->wheels->Rotate(1,10);)
				STATE_BUTTON(manualWindow, "Back", STATE_NONE)
				manualWindow.show();
		}
		if (false && wheels->CheckStall() &&
			(state == STATE_LOCATE_BALL ||
			state == STATE_BALL_LOCATED || 
			state == STATE_LOCATE_GATE || 
			state == STATE_GATE_LOCATED)
		){
			state = STATE_CRASH;
		}

		// This slows system down to 33.3 FPS
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
		else if (query == "rdrive" && tokens.size() == 4) {
			int speed = atoi(tokens[1].c_str());
			float direction = atof(tokens[2].c_str());
			int rotate = atoi(tokens[3].c_str());
			wheels->DriveRotate(speed, direction, rotate);
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
		("locate_cursor", po::value<std::string>(), "find cursor instead of ball");

	po::store(po::parse_command_line(argc, argv, desc), config);
	po::notify(config);

	if (config.count("help")) {
		std::cout << desc << std::endl;
		return false;
	}

	return true;
}
