#pragma  once
#include "types.h"
#include <opencv2/opencv.hpp>

class ObjectFinder {
private:
    ICamera *m_pCamera;

	float Vfov = 21.65; 
	int CamHeight = 345; 
	int CamAngleDev = 26; 
public:
    ObjectFinder(ICamera *pCamera){m_pCamera = pCamera; };
	std::pair<int, double> Locate(const HSVColorRange &HSVRange);
    ~ObjectFinder(){ }

};
