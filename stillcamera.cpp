#include "stillcamera.h"

StillCamera::StillCamera(const std::string &fileName)
{
	frame = cv::imread(fileName, CV_LOAD_IMAGE_COLOR);   // Read the file
}
const cv::Mat &StillCamera::Capture()
{
	return frame;
}