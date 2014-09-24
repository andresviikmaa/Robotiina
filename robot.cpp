#include "robot.h"
#include <opencv2/opencv.hpp>

int Robot::run() {
    while (true) {
        if (cv::waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            Mat frame = camera.CaptureHSV();
            
            std::cout << "esc key is pressed by user" << std::endl;
            break;
        }
    }
        return 0;
}
