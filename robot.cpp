#include "robot.h"
#include "colorcalibrator.h"
#include "autocalibrator.h"
#include "camera.h"
#include "stillcamera.h"
#include "wheelcontroller.h"
#include "coilBoard.h"
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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/filesystem.hpp>
#include "AutoPilot.h"
#include "RobotTracker.h"
#include "ImageThresholder.h"
#include "VideoRecorder.h"

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
//	std::pair<STATE, std::string>(STATE_LOCATE_BALL, "Locate Ball"),
//	std::pair<STATE, std::string>(STATE_LOCATE_GATE, "Locate Gate"),
	std::pair<STATE, std::string>(STATE_REMOTE_CONTROL, "Remote Control"),
//	std::pair<STATE, std::string>(STATE_CRASH, "Crash"),
	std::pair<STATE, std::string>(STATE_RUN, "Autopilot"),
	std::pair<STATE, std::string>(STATE_MANUAL_CONTROL, "Manual Control"),
	std::pair<STATE, std::string>(STATE_TEST_COILGUN, "Test CoilGun"),
	std::pair<STATE, std::string>(STATE_SELECT_GATE, "Select Gate"),
	std::pair<STATE, std::string>(STATE_DANCE, "Dance"),
	//	std::pair<STATE, std::string>(STATE_END_OF_GAME, "End of Game") // this is intentionally left out

};

std::map<STATE, std::string> STATE_LABELS(states, states + sizeof(states) / sizeof(states[0]));

/* BEGIN DANCE MOVES */
void dance_step(float time, float &move1, float &move2) {
	move1 = 50*sin(time/1000);
	move2 = 360 * cos(time / 1000);
}

/* END DANCE MOVES */

Robot::Robot(boost::asio::io_service &io) : Dialog("Robotiina"), io(io), camera(0), wheels(0), finder(0), coilBoard(0)
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
	if (coilBoard)
		delete coilBoard;

}

bool Robot::Launch(int argc, char* argv[])
{
	if (!ParseOptions(argc, argv)) return false;

	std::cout << "Initializing camera... " << std::endl;
	if (config.count("camera"))
		camera = new Camera(config["camera"].as<std::string>());
	else
		camera = new Camera(0);
	std::cout << "Done" << std::endl;

	if (config.count("locate_cursor"))
		finder = new MouseFinder();
	else
		finder = new ObjectFinder();

	wheels = new WheelController();
	captureFrames = config.count("capture-frames") > 0;
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
			wheels->InitWheels(io, config.count("skip-ports") > 0);
			std::cout << "Initializing Coilgun... " << std::endl;
			{
				if (config.count("skip-ports") == 0) {
					using boost::property_tree::ptree;
					ptree pt;
					read_ini("conf/ports.ini", pt);
					std::string port = pt.get<std::string>(std::to_string(ID_COILGUN));

					coilBoard = new CoilBoard(io, port);
				}
				else {
					coilBoard = new CoilGun();
				}
			}
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
	return true;
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

	std::string captureDir;
	boost::posix_time::ptime captureStart = boost::posix_time::microsec_clock::local_time();
	cv::VideoWriter *outputVideo = NULL;

	/*= "videos/" + boost::posix_time::to_simple_string(time) + "/";
	std::replace(captureDir.begin(), captureDir.end(), ':', '.');
	if (captureFrames) {
		boost::filesystem::create_directories(captureDir);
	}
	*/
	coilBoard->Start();
	AutoPilot autoPilot(wheels, coilBoard);
	//RobotTracker tracker(wheels);
	ThresholdedImages thresholdedImages;
	ImageThresholder thresholder(thresholdedImages, objectThresholds);
	ObjectFinder gate1Finder;
	ObjectFinder gate2Finder;

	VideoRecorder videoRecorder("videos/", 30, frameBGR.size());

	frameBGR = camera->Capture();
	
	std::stringstream subtitles;
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
#ifdef RECORD_AFTER_PROCESSING
		if (captureFrames) {
			videoRecorder.RecordFrame(frameBGR, subtitles.str());
		}
#endif
		frameBGR = camera->Capture();
		
#ifndef RECORD_AFTER_PROCESSING
		if (captureFrames) {
			videoRecorder.RecordFrame(frameBGR, subtitles.str());
		}
#endif

		cvtColor(frameBGR, frameHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

        if (STATE_NONE == state) {
			START_DIALOG
				STATE_BUTTON("(A)utoCalibrate objects", STATE_AUTOCALIBRATE)
				STATE_BUTTON("(M)anualCalibrate objects", STATE_CALIBRATE)
				STATE_BUTTON("(S)tart Robot", STATE_LAUNCH)
				STATE_BUTTON("(D)ance", STATE_DANCE)
				//STATE_BUTTON("(D)ance", STATE_DANCE)
				//STATE_BUTTON("(R)emote Control", STATE_REMOTE_CONTROL)
				STATE_BUTTON("Manual (C)ontrol", STATE_MANUAL_CONTROL)
				STATE_BUTTON("(T)est CoilGun", STATE_TEST_COILGUN)
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
					this->SetState(STATE_RUN);
				});
				createButton(OBJECT_LABELS[GATE2], [this]{
					this->targetGate = GATE2;
					this->SetState(STATE_RUN);
				});
			END_DIALOG
		}
		else if (STATE_LAUNCH == state) {
			if (false && targetGate == NUMBER_OF_OBJECTS) {
				std::cout << "Select target gate" << std::endl;
				SetState(STATE_SELECT_GATE);
			}
			else {
				try {
					CalibrationConfReader calibrator;
					for (int i = 0; i < NUMBER_OF_OBJECTS; i++) {
						objectThresholds[(OBJECT)i] = calibrator.GetObjectThresholds(i, OBJECT_LABELS[(OBJECT)i]);
					}
					SetState(STATE_SELECT_GATE);
				}
				catch (...){
					std::cout << "Calibration data is missing!" << std::endl;
					//TODO: display error
					SetState(STATE_NONE); // no conf
				}
			}
		}
		else if (STATE_RUN == state) {
			START_DIALOG
				createButton(std::string("Save video: ") + (captureFrames ? "on" : "off"), [this, &captureDir, &time, &videoRecorder, &frameBGR]{
					if (this->captureFrames) {
						// save old video
					}

					this->captureFrames = !this->captureFrames;
					this->captureFrames ? videoRecorder.Start() : videoRecorder.Stop();

					this->last_state = STATE_NONE; // force dialog redraw
				});
				createButton(std::string("Border detection: ") + (detectBorders ? "on" : "off"), [this]{
					this->detectBorders = !this->detectBorders;
					this->last_state = STATE_NONE; // force dialog redraw
				});
				createButton(std::string("Mouse control: ") + (dynamic_cast<MouseFinder*>(finder) == NULL ? "off" : "on"), [this]{
					bool isMouse = dynamic_cast<MouseFinder*>(finder) != NULL;
					delete this->finder;
					this->finder = isMouse ? new ObjectFinder() : new MouseFinder();
					this->last_state = STATE_NONE;
				});
				STATE_BUTTON("(B)ack", STATE_NONE)
				STATE_BUTTON("E(x)it", STATE_END_OF_GAME)
			END_DIALOG

				
			// thresholding in parallel
			thresholder.Start(frameHSV, { BALL, GATE1, GATE2, INNER_BORDER, OUTER_BORDER, FIELD });
			thresholder.WaitForStop();
			
			if (detectBorders) {
				finder->IsolateField(thresholdedImages, frameHSV, frameBGR);
			};

		
		
			ObjectPosition ballPos, gate1Pos, gate2Pos;
			//Cut out gate contour.	
			
			bool gate1Found = gate2Finder.Locate(thresholdedImages, frameHSV, frameBGR, GATE1, gate1Pos);
			bool gate2Found = gate1Finder.Locate(thresholdedImages, frameHSV, frameBGR, GATE2, gate2Pos);

			bool ballFound = finder->Locate(thresholdedImages, frameHSV, frameBGR, BALL, ballPos);
			ObjectPosition *targetGatePos = 0;
			if (targetGate == GATE1 && gate1Found) targetGatePos = &gate1Pos;
			else if(targetGate == GATE2 && gate2Found) targetGatePos = &gate2Pos;
			// else leave to NULL
			
			autoPilot.UpdateState(ballFound ? &ballPos : NULL, targetGatePos);
			
        }
		else if (STATE_MANUAL_CONTROL == state) {
			START_DIALOG
				createButton("Move Left", [this] {this->wheels->Drive(20, 90); });
				createButton("Move Right", [this]{this->wheels->Drive(20, 270); });
				createButton("Move Forward", [this]{this->wheels->Drive(20, 0); });
				createButton("Move Back", [this]{this->wheels->Drive(-20, 0); });
				createButton("Rotate Right", [this]{this->wheels->Rotate(0, 10); });
				createButton("Rotate Left", [this]{this->wheels->Rotate(1, 10); });
				STATE_BUTTON("Back", STATE_NONE)
			END_DIALOG
		}
		else if (STATE_TEST_COILGUN == state) {
			START_DIALOG
				createButton("Kick", [this] {this->coilBoard->Kick(); });
				createButton("Start tribbler", [this]{this->coilBoard->ToggleTribbler(true); });
				createButton("Stop tribbler", [this]{this->coilBoard->ToggleTribbler(false); });
				STATE_BUTTON("Back", STATE_NONE)
			END_DIALOG
		}


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
		subtitles.str("");
		subtitles << autoPilot.GetDebugInfo();
		subtitles << "|" << wheels->GetDebugInfo();


		cv::putText(frameBGR, "fps:" + std::to_string(fps), cv::Point(frameHSV.cols - 140, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));
		//assert(STATE_END_OF_GAME != state);
		cv::putText(frameBGR, "state:" + STATE_LABELS[state], cv::Point(frameHSV.cols - 140, 40), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));

		//TODO: fix putText newline thing
		std::string s(subtitles.str());
		std::replace(s.begin(), s.end(), '|', '\n');
		std::cout << s << std::endl;
		cv::putText(frameBGR, s, cv::Point(10, frameHSV.rows - 140), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 255));

		show(frameBGR);

		if (cv::waitKey(1) == 27) {
			std::cout << "exiting program" << std::endl;
			break;
		}
		frames++;

    }

	coilBoard->Stop();
	if (outputVideo != NULL) {
		delete outputVideo;
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
    else if(query == "cont") SetState(STATE_RUN);
    else if(query == "reset") SetState(STATE_NONE);
    else if(query == "exit") SetState(STATE_END_OF_GAME);
    else if (STATE_REMOTE_CONTROL == s) {
        if (query == "drive" && tokens.size() == 3) {
            int speed = atoi(tokens[1].c_str());
			double direction = atof(tokens[2].c_str());
            wheels->Drive(speed, direction);
		}
		else if (query == "rdrive" && tokens.size() == 4) {
			int speed = atoi(tokens[1].c_str());
			double direction = atof(tokens[2].c_str());
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
		("locate_cursor", "find cursor instead of ball")
		("skip-ports", "skip COM port checks")
		("save-frames", "Save captured frames to disc");

	po::store(po::parse_command_line(argc, argv, desc), config);
	po::notify(config);

	if (config.count("help")) {
		std::cout << desc << std::endl;
		return false;
	}

	return true;
}
