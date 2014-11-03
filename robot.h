#include "types.h"
#include "objectfinder.h"
#include "coilBoard.h"
#include "dialog.h"
#include <boost/atomic.hpp>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

namespace po = boost::program_options;
class WheelController;
class Robot: public Dialog {
private:
	po::variables_map config;

    ICamera *camera;
    WheelController * wheels;
	ObjectFinder *finder;
	CoilGun *coilBoard;

	HSVColorRangeMap objectThresholds;
    //STATE state = STATE_NONE;
    boost::atomic<STATE> state;
	boost::atomic<STATE> last_state;
	bool ParseOptions(int argc, char* argv[]);

	void Run();
    boost::mutex remote_mutex;
protected:
	boost::asio::io_service &io;
	OBJECT targetGate= NUMBER_OF_OBJECTS; //uselected
	bool detectBorders = false;
	bool captureFrames = false;

public:
    Robot(boost::asio::io_service &io);
	bool Launch(int argc, char* argv[]);
	~Robot();

    int GetState() {
        return state;
    }
	int GetLastState() {
		return last_state;
	}
    void SetState(STATE new_state) {
		last_state = (STATE)state;
        state = new_state;
    }
    std::string ExecuteRemoteCommand(const std::string &command);
};
