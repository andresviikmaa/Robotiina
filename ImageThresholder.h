#include "types.h"

class ImageThresholder : public ThreadedClass
{
protected:
	ThresholdedImages &thresholdedImages;
	HSVColorRangeMap &objectMap;
public:
	void Start(cv::Mat &frameHSV, std::vector<OBJECT> objectList) {
		for (auto &object : objectList) {
			threads.create_thread([&frameHSV, object, this]{
				auto r = objectMap[object];
				inRange(frameHSV, cv::Scalar(r.hue.low, r.sat.low, r.val.low), cv::Scalar(r.hue.high, r.sat.high, r.val.high), thresholdedImages[object]);
			});
		}
	}
	ImageThresholder(ThresholdedImages &images, HSVColorRangeMap &objectMap) : ThreadedClass("ImageThresholder"), thresholdedImages(images), objectMap(objectMap){};
	~ImageThresholder(){};

	void Run(){};
};

