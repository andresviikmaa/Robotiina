#include "robot.h"
#include "colorcalibrator.h"
#include "autocalibrator.h"
#include "camera.h"
#include "stillcamera.h"
#include "wheelcontroller.h"
#include "objectfinder.h"
#include "MouseFinder.h"
#include "dialog.h"
#include "wheel.h"
#include "ComPortScanner.h"

#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/tuple/tuple.hpp>


#define STATE_BUTTON(name, new_state) \
createButton(name, [&](){ this->state = new_state; });
#define BUTTON(name, function_body) \
createButton(name, [&]() function_body);

std::pair<OBJECT, std::string> objects[] = {
	std::pair<OBJECT, std::string>(BALL, "Ball"),
	std::pair<OBJECT, std::string>(GATE1, "Blue Gate"),
	std::pair<OBJECT, std::string>(GATE2, "Yellow Gate"),
	std::pair<OBJECT, std::string>(FIELD, "Field"),
    std::pair<OBJECT, std::string>(INNER_BORDER, "Inner Border"),
    std::pair<OBJECT, std::string>(OUTER_BORDER, "Outer Border"),

};

std::map<OBJECT, std::string> OBJECT_LABELS(objects, objects + sizeof(objects) / sizeof(objects[0]));

Robot::Robot(boost::asio::io_service &io) : Dialog("Robotiina"), io(io), camera(0), wheels(0), finder(0)
{
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
void Robot::CalibrateObject(const cv::Mat &image, bool autoCalibrate/* = false*/)
{
	/*
    ColorCalibrator* calibrator = autoCalibrate ? new AutoCalibrator() : new ColorCalibrator();
    calibrator->LoadImage(image);
		return calibrator->GetObjectThresholds(i, OBJECT_LABELS[(OBJECT) i]);
            //BUTTON(OBJECT_LABELS[(OBJECT) i], i )

    }

    STATE_BUTTON("Back", STATE_NONE)
    delete calibrator;
	*/
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

	bool portsOk = false;
	if (config.count("skip-ports")) {
		std::cout << "Skiping COM port check" << std::endl;
		portsOk = true;
	}
	else {
		std::cout << "Checking COM ports... " << std::endl;
		{ // new scope for scanner variable
			ComPortScanner scanner;
			if ((portsOk = scanner.Verify(io)) == false){
				std::cout << "Chek failed, rescanning all ports" << std::endl;
				portsOk = scanner.Scan(io);
			}
		}
		std::cout << "Done" << std::endl;
	}
	if (portsOk) {
		std::cout << "Initializing Wheels... " << std::endl;
		try {
			wheels = new WheelController(io);
		}
		catch (...) {
                throw;
			//throw std::runtime_error("error to open wheel port(s)");
		}
		std::cout << "Done" << std::endl;
	}
	else {
		throw std::runtime_error("unable find wheels use \"--skip-ports\" parameter to launch without wheels ");
	}

	std::cout << "Starting Robot" << std::endl;
    Run();
}
void Robot::Run()
{
 
	double fps = 0;
	int frames = 0;
	boost::posix_time::ptime lastStepTime;

	boost::posix_time::time_duration dt;
	boost::posix_time::ptime time = boost::posix_time::second_clock::local_time();

	cv::Mat frameBGR, frameHSV;
	while (true)
    {
		time = boost::posix_time::second_clock::local_time();
		boost::posix_time::time_duration::tick_type dt = (time - lastStepTime).total_milliseconds();
		if (dt > 1000) {
			fps = 1000.0 * frames / dt;
			lastStepTime = time;
			frames = 0;
		}
		frameBGR = camera->Capture();
		cvtColor(frameBGR, frameHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

        if (STATE_NONE == state) {
			clearButtons();
			STATE_BUTTON("(A)utoCalibrate objects", STATE_AUTOCALIBRATE)
				STATE_BUTTON("(M)anualCalibrate objects", STATE_CALIBRATE)
				STATE_BUTTON("(S)tart Robot", STATE_LAUNCH)
				//			STATE_BUTTON("(R)emote Control", STATE_REMOTE_CONTROL)
				//			STATE_BUTTON("Manual (C)ontrol", STATE_MANUAL_CONTROL)
				STATE_BUTTON("E(x)it", STATE_END_OF_GAME)

		}
		else if (STATE_CALIBRATE == state || STATE_AUTOCALIBRATE == state) {
			clearButtons();
			for (int i = 0; i < NUMBER_OF_OBJECTS; i++) {
				// objectThresholds[(OBJECT) i] = calibrator->GetObjectThresholds(i, OBJECT_LABELS[(OBJECT) i]);
				createButton(OBJECT_LABELS[(OBJECT)i], [this, i, &frameBGR]{
					ColorCalibrator* calibrator = STATE_AUTOCALIBRATE == state ? new AutoCalibrator() : new ColorCalibrator();
					calibrator->LoadImage(frameBGR);
					calibrator->GetObjectThresholds(i, OBJECT_LABELS[(OBJECT)i]);
					delete calibrator;
				});
			}
			STATE_BUTTON("BACK", STATE_NONE)
		}
		else if (STATE_LAUNCH == state) {
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
		else if(STATE_CRASH == state){
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
		else if(STATE_LOCATE_BALL == state) {
			
			finder->IsolateField(objectThresholds[INNER_BORDER], objectThresholds[OUTER_BORDER], objectThresholds[GATE1], objectThresholds[GATE2], frameHSV, frameBGR);
			cv::Point3d location = finder->Locate(objectThresholds[BALL], frameHSV, frameBGR, false);
			if (location.x == -1 && location.y == -1 && location.z == -1) /* Ball not found */
            {
                wheels->Rotate(1, 10);
            }
			if (location.x != -1 && location.y != -1) { /*Ball found*/
				bool ballInTribbler = wheels->DriveToBall(location.x, //distance
														location.y,	//horizontal dev
														location.z, //angle
														300); //desired distance
				if (ballInTribbler){
					state = STATE_LOCATE_GATE;
				}
			}
            
        }
		else if (STATE_LOCATE_GATE == state)
		{
			//			finder->IsolateField(objectThresholds[INNER_BORDER], objectThresholds[OUTER_BORDER], objectThresholds[GATE1], objectThresholds[GATE2], frameHSV, frameBGR);
			bool ballInTribbler = true; // fix me
			if (!ballInTribbler) {
				state = STATE_LOCATE_BALL;

			}
			else {
				cv::Point3d location = finder->Locate(objectThresholds[GATE1], frameHSV, frameBGR, true);
				//If not found
				if (location.x == -1 && location.y == -1 && location.z == -1){
					wheels->Rotate(1, 10);
				}
				else{
					//TODO: kick ball
					state = STATE_LOCATE_BALL;
				}
			}
		}
		/*
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
		*/
		else if (STATE_END_OF_GAME == state) {
			break;
		}

		if (false && wheels->CheckStall() &&
			(state == STATE_LOCATE_BALL ||
			state == STATE_LOCATE_GATE))
		{
			state = STATE_CRASH;
		}

		cv::putText(frameBGR, "fps:" + std::to_string(fps), cv::Point(frameHSV.cols - 140, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		cv::putText(frameBGR, "state:" + std::to_string(state), cv::Point(frameHSV.cols - 140, 40), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));

		show(frameBGR);
		if (cv::waitKey(1) == 27) {
			std::cout << "exiting program" << std::endl;
			break;
		}
		frames++;

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
		("locate_cursor", po::value<std::string>(), "find cursor instead of ball")
		("skip-ports", po::value<std::string>(), "skip COM port checks");

	po::store(po::parse_command_line(argc, argv, desc), config);
	po::notify(config);

	if (config.count("help")) {
		std::cout << desc << std::endl;
		return false;
	}

	return true;
}
