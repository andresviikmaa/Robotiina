#include "types.h"
#include "objectfinder.h"

#include <boost/atomic.hpp>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
namespace po = boost::program_options;
class WheelController;
class Robot {
private:
	po::variables_map config;

    ICamera *camera;
    WheelController * wheels;
    std::map<OBJECT, HSVColorRange> objectThresholds;
    //STATE state = STATE_NONE;
    boost::atomic<STATE> state;
    void CalibrateObjects(bool autoCalibrate = false);
	bool ParseOptions(int argc, char* argv[]);

	void Run();
    boost::mutex remote_mutex;

protected:
	boost::asio::io_service &io;

public:
    Robot(boost::asio::io_service &io);
	bool Launch(int argc, char* argv[]);
	~Robot();

    int GetState() {
        return state;
    }
    void SetState(STATE state) {
        this->state = state;
    }
    std::string ExecuteRemoteCommand(const std::string &command);
};
