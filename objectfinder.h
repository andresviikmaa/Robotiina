#pragma  once
#include "types.h"
#include <opencv2/opencv.hpp>

class ObjectFinder {
private:
    ICamera *m_pCamera;
public:
    ObjectFinder(ICamera *pCamera){m_pCamera = pCamera; };
	std::pair<int, double> Locate(const HSVColorRange &HSVRange);
    ~ObjectFinder(){ }

};
