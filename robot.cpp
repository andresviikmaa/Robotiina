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
	/* Experiment by Andres
	cv::Point2f speed; // speed
	//cv::Point2f pos(0, 0); // robot position
	cv::Point2f tc(300, 300);

	cv::Mat track, track2, track3, track_rotated;
	cv::Mat track_empty(tc.x * 2, tc.y * 2, CV_8UC3, cv::Scalar::all(0));
	track_empty.copyTo(track);
	track_empty.copyTo(track2);



	cv::Point2f pos(90, 0);
	cv::Point2f new_pos(60, 0);

	//cv::line(track, tc, pos + tc, cv::Scalar(0, 0, 255));
	float angle = atan(pos.x / pos.y) * (180 / PI);
	//cv::line(track, tc, new_pos + tc, cv::Scalar(255, 0, 0));

	cv::Mat rot_mat = cv::getRotationMatrix2D(cv::Point2f(0, 0), angle, 1.0);

	double data[3] = { new_pos.x, new_pos.y, 0 };
	cv::Mat pos_mat = cv::Mat(3, 1, CV_64FC1, data);
	cv::Mat new_pos2 = rot_mat * pos_mat;

	new_pos = cv::Point2f(new_pos2.at<double>(cv::Point(0, 0)), new_pos2.at<double>(cv::Point(0, 1)));

	cv::line(track, tc + pos, tc + pos + new_pos, cv::Scalar(0, 255, 0));
	cv::imshow("track", track);


	cv::imshow("track2", track3);

	cv::waitKey();
	return false;
	*/
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
			wheels = new WheelController(io, config.count("skip-ports") > 0);
			std::cout << "Initializing Coilgun... " << std::endl;
			{
				using boost::property_tree::ptree;
				ptree pt;
				read_ini("conf/ports.ini", pt);
				std::string port = pt.get<std::string>(std::to_string(ID_COILGUN));

				/*coilBoard = new CoilBoard(io, port);*/
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

	std::string captureDir = "videos/" + boost::posix_time::to_simple_string(time) + "/";
	std::replace(captureDir.begin(), captureDir.end(), ':', '.');
	if (captureFrames) {
		boost::filesystem::create_directories(captureDir);
	}
	cv::Point3f speed; // speed, x, y and w
	cv::Point3f pos(0, 0, 0); // robot position and heading in 2D

	cv::Mat track, track_rotated;
	cv::Point2f tc(300, 300);
	cv::Mat track_empty(tc.x * 2, tc.y * 2, CV_8UC1, cv::Scalar::all(0));
	track_empty.copyTo(track);
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
		/*
		if (pos.x < -tc.x) { pos.x += tc.x; track_empty.copyTo(track); };
		if (pos.y < -tc.y) { pos.y += tc.y; track_empty.copyTo(track); };
		if (pos.x > tc.x) { pos.x -= tc.x; track_empty.copyTo(track); };
		if (pos.y > tc.y) { pos.y -= tc.y; track_empty.copyTo(track); };
		*/
		
		//cv::line(track, tc, new_pos + tc, cv::Scalar(255, 0, 0));
		/*
		telliskivi2:
		orientation = Math::floatModulus(orientation + omega * dt, Math::TWO_PI);
		x += (velocityX * Math::cos(orientation) - velocityY * Math::sin(orientation)) * dt;
		y += (velocityX * Math::sin(orientation) + velocityY * Math::cos(orientation)) * dt;
		*/
		float _dt = dt / 1000; 
		cv::Point3f old_pos(pos);
		pos.z = fmod(pos.z + (speed.z * _dt), 360);
		
		pos.x += (speed.x * cos(pos.z / (2*PI)) - speed.y * sin(pos.z / (2*PI))) * dt;
		pos.y += (speed.z * sin(pos.z / (2*PI)) + speed.y * cos(pos.z / (2*PI))) * dt;

/*	same as above, but with matrix operation	
		cv::Mat rot_mat = cv::getRotationMatrix2D(cv::Point2f(0, 0), heading, 1.0);
		double data[3] = { new_pos.x, new_pos.y, 0 };
		cv::Mat pos_mat = cv::Mat(3, 1, CV_64FC1, data);
		cv::Mat new_pos2 = rot_mat * pos_mat;
		new_pos = cv::Point2f(new_pos2.at<double>(cv::Point(0, 0)), new_pos2.at<double>(cv::Point(0, 1)));
*/

		//cv::line(track, tc + cv::Point2i(old_pos.x,old_pos.y), tc + cv::Point2i(pos.x,pos.y), cv::Scalar(255, 255, 255));
		cv::imshow("track", track);
		
		frameBGR = camera->Capture();
		if (captureFrames) {
			std::string frameName = captureDir + boost::posix_time::to_simple_string(time) + ".jpg";
			std::replace(frameName.begin(), frameName.end(), ':', '.');
			cv::imwrite(frameName , frameBGR);
		}
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
					this->SetState(STATE_LOCATE_BALL);
				});
				createButton(OBJECT_LABELS[GATE2], [this]{
					this->targetGate = GATE2;
					this->SetState(STATE_LOCATE_BALL);
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
		else if(STATE_CRASH == state){
			//Backwards
			speed = wheels->Drive(50, 180);
		        std::chrono::milliseconds dura(1000);
			std::this_thread::sleep_for(dura);
			wheels->Stop();
			//Turn a littlebit
			wheels->Rotate(1, 20);
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
				createButton(std::string("Save video: ") + (captureFrames ? "on" : "off"), [this, &captureDir, &time]{
					this->captureFrames = !this->captureFrames;
					if (this->captureFrames) {
						captureDir = "videos/" + boost::posix_time::to_simple_string(time) + "/";
						std::replace(captureDir.begin(), captureDir.end(), ':', '.');
						boost::filesystem::create_directories(captureDir);
					}
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
			if (detectBorders) {
				finder->IsolateField(objectThresholds[INNER_BORDER], objectThresholds[OUTER_BORDER], objectThresholds[GATE1], objectThresholds[GATE2], frameHSV, frameBGR);
			}
			cv::Point3d location = finder->Locate(objectThresholds[BALL], frameHSV, frameBGR, false, objectThresholds[INNER_BORDER], objectThresholds[OUTER_BORDER]);
			time = boost::posix_time::microsec_clock::local_time();
			if (location.x == -1 && location.y == -1 && location.z == -1) /* Ball not found */
            {
				boost::posix_time::time_duration::tick_type rotateDuration = (time - rotateTime).total_milliseconds();
				if (rotateDuration >= 500){
					wheels->Stop();
					if (rotateDuration >= 600){
						rotateTime = time; //reset
					}	
				}
				else{
					wheels->Rotate(1, 30);
				}
            }
			else { /*Ball found*/
				rotateTime = time; //reset timer
				speed = wheels->DriveToBall(location.x, //distance
														location.y,	//horizontal dev
														location.z, //angle
														210
														/*,coilBoard*/); //desired distance
				if (sqrt(pow(speed.x, 2) + pow(speed.y, 2)) < 0.1){
					SetState(STATE_LOCATE_GATE);
				}
			}
            
        }
		else if (STATE_LOCATE_GATE == state)
		{
			//finder->IsolateField(objectThresholds[INNER_BORDER], objectThresholds[OUTER_BORDER], objectThresholds[GATE1], objectThresholds[GATE2], frameHSV, frameBGR);
			/*
			bool ballInTribbler = coilBoard->BallInTribbler();
			*/
			bool ballInTribbler = true;
			if (!ballInTribbler) {
				SetState(STATE_LOCATE_BALL);
			}
			else {
				cv::Point3d location = finder->Locate(objectThresholds[targetGate], frameHSV, frameBGR, true, objectThresholds[INNER_BORDER], objectThresholds[OUTER_BORDER]);
				//If not found
				if (location.x == -1 && location.y == -1 && location.z == -1){
					wheels->Rotate(1, 15);
				}
				else{
					//TODO: kick ball
					/*
					coilBoard->Kick();
					*/
					speed = wheels->DriveToBall(location.x, //distance
						location.y,	//horizontal dev
						location.z, //angle
						500//desired distance
						/*,coilBoard*/);
					if (sqrt(pow(speed.x, 2) + pow(speed.y, 2)) < 0.1){
						SetState(STATE_LOCATE_BALL);
					}
				}
			}
		}
		/*
        else if(STATE_REMOTE_CONTROL == state) {
			Dialog launchWindow("Remote Control Mode Enabed", CV_WINDOW_AUTOSIZE);
			STATE_BUTTON(launchWindow, "Back", STATE_NONE)
			launchWindow.show();

        }
		*/
		else if (STATE_MANUAL_CONTROL == state) {
			START_DIALOG
				createButton("Move Left", [this, &speed] {speed = this->wheels->Drive(20, 90); });
				createButton("Move Right", [this]{this->wheels->Drive(20, 270); });
				createButton("Move Forward", [this]{this->wheels->Drive(20, 0); });
				createButton("Move Back", [this]{this->wheels->Drive(-20, 0); });
				createButton("Rotate Right", [this]{this->wheels->Rotate(0, 10); });
				createButton("Rotate Left", [this]{this->wheels->Rotate(1, 10); });
				STATE_BUTTON("Back", STATE_NONE)
			END_DIALOG
		}

		else if (STATE_DANCE == state) {
			float move1, move2;
			dance_step(((float)(time - epoch).total_milliseconds()), move1, move2);
			speed = wheels->Drive(move1, move2);
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
