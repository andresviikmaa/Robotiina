#include "colorcalibrator.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

ColorCalibrator::ColorCalibrator()
{
	range = {{0,179},{0,255},{0,255}};
    cvNamedWindow("ColorCalibrator", CV_WINDOW_AUTOSIZE); //create a window called "Control"
    cvCreateTrackbar("LowH", "ColorCalibrator", &range.hue.low, range.hue.high); //Hue (0 - 179)
    cvCreateTrackbar("HighH", "ColorCalibrator", &range.hue.high, range.hue.high);


    cvCreateTrackbar("LowS", "ColorCalibrator", &range.sat.low, range.sat.high); //Saturation (0 - 255)
    cvCreateTrackbar("HighS", "ColorCalibrator", &range.sat.high, range.sat.high);


    cvCreateTrackbar("LowV", "ColorCalibrator", &range.val.low, range.val.high); //Value (0 - 255)
    cvCreateTrackbar("HighV", "ColorCalibrator", &range.val.high, range.val.high);


};

void ColorCalibrator::LoadImage(const cv::Mat &image, int numberOfObjects)
{
    this->image = image;
};
HSVColorRange ColorCalibrator::GetObjectThresholds (int index, const std::string &name)
{
	try
	{
		LoadConf(name);
	}
	catch (...){};
    cv::Mat imgThresholded, imgHSV;
    cvtColor(image, imgHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV



    while (true)
    {
        cv::inRange(imgHSV, cv::Scalar(range.hue.low, range.sat.low, range.val.low), cv::Scalar(range.hue.high, range.sat.high, range.val.high), imgThresholded); //Threshold the image

        cv::imshow(name.c_str(), imgThresholded); //show the thresholded image

        if (cv::waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
			cvDestroyWindow(name.c_str());
			std::cout << "esc key is pressed by user" << std::endl;
			SaveConf(name);
			return range;
		}
    }

};
void ColorCalibrator::SaveConf(const std::string &name){
	using boost::property_tree::ptree;

	ptree pt;
	pt.put("hue.low", range.hue.low);
	pt.put("hue.high", range.hue.high);
	pt.put("sat.low", range.sat.low);
	pt.put("sat.high", range.sat.high);
	pt.put("val.low", range.val.low);
	pt.put("val.high", range.val.high);


	write_ini(std::string("conf/") +name +".ini", pt);
}
void ColorCalibrator::LoadConf(const std::string &name){
	using boost::property_tree::ptree;

	ptree pt;
	read_ini(std::string("conf/") + name + ".ini", pt);
	range.hue.low = pt.get<int>("hue.low");
	range.hue.high = pt.get<int>("hue.high");
	range.sat.low = pt.get<int>("sat.low");
	range.sat.high = pt.get<int>("sat.high");
	range.val.low = pt.get<int>("val.low");
	range.val.high = pt.get<int>("val.high");

}

ColorCalibrator::~ColorCalibrator(){
    cvDestroyWindow("ColorCalibrator");
}
