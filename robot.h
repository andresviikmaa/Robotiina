#include "camera.h"
#include "objectfinder.h"

class Robot {
protected:
    Camera camera;
    ObjectFinder objectFinder;
public:
    Robot(){ };
    int run();
    ~Robot(){ }
};
