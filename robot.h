#include "types.h"
#include "objectfinder.h"

class Robot {
private:
    ICamera *camera;
    std::map<OBJECT, HSVColorRange> objectThresholds;
    STATE state = LOCATE_BALL;
    void CalibrateObjects();
public:
    Robot();
    int run();
    ~Robot();
};
