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
#include <boost/timer/timer.hpp>


#define STATE_BUTTON(name, new_state) \
createButton(name, [&](){ this->SetState(new_state); });
#define BUTTON(name, function_body) \
createButton(name, [&]() function_body);
#define START_DIALOG if (state != last_state) { \
clearButtons();
#define END_DIALOG }

std::pair<OBJECT, std::string> objects[] = {
	std::pair<OBJECT, std::string>(BALL, "Ball"),
	std::pair<OBJECT, std::string>(GATE1, "Blue Gate"),
	std::pair<OBJECT, std::string>(GATE2, "Yellow Gate"),
	std::pair<OBJECT, std::string>(FIELD, "Field"),
    std::pair<OBJECT, std::string>(INNER_BORDER, "Inner Border"),
	std::pair<OBJECT, std::string>(OUTER_BORDER, "Outer Border"),
//	std::pair<OBJECT, std::string>(NUMBER_OF_OBJECTS, "") // this is intentionally left out

};

std::map<OBJECT, std::string> OBJECT_LABELS(objects, objects + sizeof(objects) / sizeof(objects[0]));

std::pair<STATE, std::string> states[] = {
	std::pair<STATE, std::string>(STATE_NONE, "None"),
	std::pair<STATE, std::string>(STATE_AUTOCALIBRATE, "Autocalibrate"),
	std::pair<STATE, std::string>(STATE_CALIBRATE, "Manual calibrate"),
	std::pair<STATE, std::string>(STATE_LAUNCH, "Launch"),
	std::pair<STATE, std::string>(STATE_LOCATE_BALL, "Locate Ball"),
	std::pair<STATE, std::string>(STATE_LOCATE_GATE, "Locate Gate"),
	std::pair<STATE, std::string>(STATE_REMOTE_CONTROL, "Remote Control"),
	std::pair<STATE, std::string>(STATE_CRASH, "Crash"),
	std::pair<STATE, std::string>(STATE_MANUAL_CONTROL, "Manual Control"),
	std::pair<STATE, std::string>(STATE_SELECT_GATE, "Select Gate"),
	std::pair<STATE, std::string>(STATE_DANCE, "Dance"),
	//	std::pair<STATE, std::string>(STATE_END_OF_GAME, "End of Game") // this is intentionally left out

};

std::map<STATE, std::string> STATE_LABELS(states, states + sizeof(states) / sizeof(states[0]));

/* BEGIN DANCE MOVES */
void dance_step(float time, float &move1, float &move2) {
	move1 = 50*sin(time/1000);
	move2 = 10;// 180 * cos(time / 10000);
}

/* END DANCE MOVES */

Robot::Robot(boost::asio::io_service &io) : Dialog("Robotiina"), io(io), camera(0), wheels(0), finder(0)
{
	
	last_state = STATE_END_OF_GAME;
	state = STATE_NONE;
    //wheels = new WheelController(io);
	assert(OBJECT_LABELS.size() == NUMBER_OF_OBJECTS);
	assert(STATE_LABELS.size() == STATE_END_OF_GAME);
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
	//timer for rotation measure
	boost::posix_time::ptime lastStepTime;	
	boost::posix_time::time_duration dt;	
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::ptime epoch = boost::posix_time::microsec_clock::local_time();

	boost::posix_time::ptime rotateTime = time;
	boost::posix_time::time_duration rotateDuration;
	cv::Mat frameBGR, frameHSV;
	while (true)
    {
		
		time = boost::posix_time::microsec_clock::local_time();
		boost::posix_time::time_duration::tick_type dt = (time - lastStepTime).total_milliseconds();
		boost::posix_time::time_duration::tick_type rotateDuration = (time - rotateTime).total_milliseconds();

		if (dt > 1000) {
			fps = 1000.0 * frames / dt;
			lastStepTime = time;
			frames = 0;
		}
		frameBGR = camera->Capture();
		cvtColor(frameBGR, frameHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

        if (STATE_NONE == state) {
			START_DIALOG
				STATE_BUTTON("(A)utoCalibrate objects", STATE_AUTOCALIBRATE)
				STATE_BUTTON("(M)anualCalibrate objects", STATE_CALIBRATE)
				STATE_BUTTON("(S)tart Robot", STATE_LAUNCH)
				STATE_BUTTON("(D)ance", STATE_DANCE)
				//STATE_BUTTON("(D)ance", STATE_DANCE)
				//			STATE_BUTTON("(R)emote Control", STATE_REMOTE_CONTROL)
				//			STATE_BUTTON("Manual (C)ontrol", STATE_MANUAL_CONTROL)
				STATE_BUTTON("E(x)it", STATE_END_OF_GAME)
			END_DIALOG
		}
		else if (STATE_CALIBRATE == state || STATE_AUTOCALIBRATE == state) {
			START_DIALOG
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
			END_DIALOG
		}
		else if (STATE_SELECT_GATE == state) {
			START_DIALOG
				createButton(OBJECT_LABELS[GATE1], [this]{
					this->targetGate = GATE1;
					this->SetState(STATE_LAUNCH);
				});
				createButton(OBJECT_LABELS[GATE2], [this]{
					this->targetGate = GATE2;
					this->SetState(STATE_LAUNCH);
				});
			END_DIALOG
		}
		else if (STATE_LAUNCH == state) {
			if (targetGate == NUMBER_OF_OBJECTS) {
				std::cout << "Select target gate" << std::endl;
				SetState(STATE_SELECT_GATE);
			}
			else {
				try {
					CalibrationConfReader calibrator;
					for (int i = 0; i < NUMBER_OF_OBJECTS; i++) {
						objectThresholds[(OBJECT)i] = calibrator.GetObjectThresholds(i, OBJECT_LABELS[(OBJECT)i]);
					}
					SetState(STATE_LOCATE_BALL);
				}
				catch (...){
					std::cout << "Calibration data is missing!" << std::endl;
					//TODO: display error
					SetState(STATE_NONE); // no conf
				}
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
				SetState(STATE_LOCATE_BALL);
			}            
		}
		else if(STATE_LOCATE_BALL == state) {
			START_DIALOG
				STATE_BUTTON("Go to Locate Gate", STATE_LOCATE_GATE)
				createButton(std::string("Border detection: ") + (DetectBorders ? "on" : "off"), [this]{
					this->DetectBorders = !this->DetectBorders;
				}); 
				STATE_BUTTON("(B)ack", STATE_NONE)
				STATE_BUTTON("E(x)it", STATE_END_OF_GAME)
			END_DIALOG
			if (DetectBorders) {
				finder->IsolateField(objectThresholds[INNER_BORDER], objectThresholds[OUTER_BORDER], objectThresholds[GATE1], objectThresholds[GATE2], frameHSV, frameBGR);
			}
			cv::Point3d location = finder->Locate(objectThresholds[BALL], frameHSV, frameBGR, false);
			time = boost::posix_time::microsec_clock::local_time();
			if (location.x == -1 && location.y == -1 && location.z == -1) /* Ball not found */
            {
				boost::posix_time::time_duration::tick_type rotateDuration = (time - rotateTime).total_milliseconds();
				if (rotateDuration >= 700){
					wheels->Stop();
					std::cout << "i'm waiting "<< rotateDuration << "\n";
					if (rotateDuration >= 800){
						rotateTime = time; //reset
						std::cout << "reset" << "\n";
					}	
				}
				else{
					wheels->Rotate(1, 80);
					std::cout << "i'm rotating " << rotateDuration << "\n";
				}
            }

			if (location.x != -1 && location.y != -1) { /*Ball found*/
				rotateTime = time; //reset timer
				bool ballInTribbler = wheels->DriveToBall(location.x, //distance
														location.y,	//horizontal dev
														location.z, //angle
														350); //desired distance
				if (ballInTribbler){
					SetState(STATE_LOCATE_GATE);
				}
			}
            
        }
		else if (STATE_LOCATE_GATE == state)
		{
			//finder->IsolateField(objectThresholds[INNER_BORDER], objectThresholds[OUTER_BORDER], objectThresholds[GATE1], objectThresholds[GATE2], frameHSV, frameBGR);
			bool ballInTribbler = true; // fix me
			if (!ballInTribbler) {
				SetState(STATE_LOCATE_BALL);
			}
			else {
				cv::Point3d location = finder->Locate(objectThresholds[targetGate], frameHSV, frameBGR, true);
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
		else if (STATE_DANCE == state) {
			float move1, move2;
			dance_step(((float)(time - epoch).total_milliseconds()), move1, move2);
			wheels->Drive(move1, move2);
			cv::putText(frameBGR, "move1:" + std::to_string(move1), cv::Point(frameHSV.cols - 140, 120), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
			cv::putText(frameBGR, "move2:" + std::to_string(move2), cv::Point(frameHSV.cols - 140, 140), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		}
		else if (STATE_END_OF_GAME == state) {
			break;
		}

		if (false && wheels->CheckStall() &&
			(state == STATE_LOCATE_BALL ||
			state == STATE_LOCATE_GATE))
		{
			SetState(STATE_CRASH);
		}

		cv::putText(frameBGR, "fps:" + std::to_string(fps), cv::Point(frameHSV.cols - 140, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		assert(STATE_END_OF_GAME != state);
		cv::putText(frameBGR, "state:" + STATE_LABELS[state], cv::Point(frameHSV.cols - 140, 40), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));

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
