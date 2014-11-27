
#include "VideoRecorder.h"


VideoRecorder::VideoRecorder(const std::string &outputDir, int fps, const cv::Size &frameSize) : outputDir(outputDir), fps(fps), frameSize(frameSize)
{
}


VideoRecorder::~VideoRecorder()
{
	Stop();
}

void VideoRecorder::Start()
{
	Stop();
	outputVideo = new cv::VideoWriter();
	subtitles = new std::ofstream();
#ifdef WIN32
	int ex = -1;
#else
	int ex = CV_FOURCC('x','v','i','d'); //CV_FOURCC('F', 'M', 'P', '4');
#endif
	boost::posix_time::ptime captureStart = boost::posix_time::microsec_clock::local_time();
	fileName = outputDir + boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time());
	std::replace(fileName.begin(), fileName.end(), ':', '.');

	std::cout << "Save video: " << fileName << ".avi" << ex << fps << frameSize << std::endl;
 	outputVideo->open(fileName + ".avi", ex, fps, frameSize, true);
	if (!outputVideo->isOpened())
	{
		std::cout << "Could not open the output video for write: " << fileName << std::endl;
	}
	subtitles->open(fileName + ".sub");
	frameCounter = 1;


}
void VideoRecorder::Stop()
{
	if (outputVideo != NULL) {
		delete outputVideo;
		outputVideo = NULL;
	}
	if (subtitles != NULL) {
		subtitles->close();
		delete subtitles;
		subtitles = NULL;
	}
}
void VideoRecorder::RecordFrame(const cv::Mat &frame, const std::string subtitle)
{
	*outputVideo << frame;
	*subtitles << "{" << frameCounter << "}{" << (frameCounter) << "}" << " frame " << frameCounter << ": " << boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time()) << "|" << subtitle << "\r\n";
	frameCounter++;
}
