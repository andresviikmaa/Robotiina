#include "autocalibrator.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

AutoCalibrator::AutoCalibrator()
{
    range = {{0,179},{0,255},{0,255}};
};
void AutoCalibrator::LoadImage(const cv::Mat &image)
{
    ColorCalibrator::LoadImage(image);
    float data[6][3] = {{1, 0, 0/*blue*/}, {0, 0, 1 /* orange*/}, {1 ,1, 0 /* yellow*/}, {0,1, 0}/*green*/, {1,1,1}, {0,0,0}};
    centers = cv::Mat(5,1, CV_32FC3, &data); //BGR
    DetectThresholds(6);
};

HSVColorRange AutoCalibrator::GetObjectThresholds (int index, const std::string &name)
{
    cv::imshow(name.c_str(), clustered); //show the thresholded image
    cv::setMouseCallback(name.c_str(), [](int event, int x, int y, int flags, void* self) {
        if (event==cv::EVENT_LBUTTONUP){
            ((AutoCalibrator*)self)->mouseClicked(x, y);
        }
    }, this);


    done = false;
    while (!done)
    {
        if (cv::waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
            std::cout << "esc key is pressed by user" << std::endl;
            done = true;
        }
    }
    cvDestroyWindow(name.c_str());
    SaveConf(name);
    return range;

};

void AutoCalibrator::mouseClicked(int x, int y) {
    cv::Mat imgHSV(image);
    cvtColor(imgHSV,image,CV_BGR2HSV);

    int label = bestLabels.at<int>(y*image.rows + x);
    range =  {{179,0},{255,0},{255,0}} /* reverse initial values for min/max to work*/;
    for(int i=0; i<image.cols*image.rows; i++) {
        if(bestLabels.at<int>(i) == label){
            int hue = imgHSV.at<cv::Vec3b>(i).val[0];
            int sat = imgHSV.at<cv::Vec3b>(i).val[1];
            int val = imgHSV.at<cv::Vec3b>(i).val[2];

            range.hue.low = std::min(range.hue.low, hue);
            range.hue.high = std::max(range.hue.high, hue);
            range.sat.low = std::min(range.sat.low, sat);
            range.sat.high = std::max(range.sat.high, sat);
            range.val.low = std::min(range.val.low, val);
            range.val.high = std::max(range.val.high, val);
        }
    }
    done = true;

}
AutoCalibrator::~AutoCalibrator(){
//    cvDestroyWindow("ColorCalibrator");
}

void AutoCalibrator::DetectThresholds(int number_of_objects){
    cv::Mat img(image);
    //cvtColor(img,image,CV_BGR2HSV);
    int origRows = img.rows;
    std::cout << "original image is: " << img.rows << "x" << img.cols << std::endl;
    cv::Mat colVec = img.reshape(1, img.rows*img.cols); // change to a Nx3 column vector
    std::cout << "colVec is of size: " << colVec.rows << "x" << colVec.cols << std::endl;
    cv::Mat colVecD;
    int attempts = 5;

    double eps = 0.001;
    colVec.convertTo(colVecD, CV_32FC3, 1.0/255.0); // convert to floating point
    double compactness = cv::kmeans(colVecD, number_of_objects, bestLabels,
            cv::TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, attempts, eps),
            attempts, cv::KMEANS_USE_INITIAL_LABELS, centers);
    cv::Mat labelsImg = bestLabels.reshape(1, origRows); // single channel image of labels
    std::cout << "Compactness = " << compactness << std::endl;
    clustered = cv::Mat(1, img.rows*img.cols , CV_32FC3, 255);

    std::cout << centers << std::endl;
    std::cout << ">" << " " << centers.at<cv::Point3f>(0) << " " << bestLabels.at<int>(3) << std::endl;
    std::cout << centers.at<float>(bestLabels.at<int>(0), 1) << std::endl;
    std::cout << img.cols*img.rows << ":" << bestLabels.rows << std::endl;
    for(int i=0; i<img.cols*img.rows; i++) {
        clustered.at<cv::Point3f>(i) = centers.at<cv::Point3f>(bestLabels.at<int>(i));
    }

    //clustered.convertTo(clustered, CV_8UC3, 255);

    clustered = clustered.reshape(3, img.rows);
    std::cout << "clustered image is: " << clustered.rows << "x" << clustered.cols << std::endl;

    //cvtColor(clustered,img,CV_HSV2BGR);



}
