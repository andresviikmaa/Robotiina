
#include "ThreadedClass.h"


ThreadedClass::ThreadedClass()
{
	stop_thread = false;
}

void ThreadedClass::Start()
{
	threads.create_thread(boost::bind(&ThreadedClass::Run, this));
};
void ThreadedClass::WaitForStop()
{
	stop_thread = true;
	threads.join_all();
};

ThreadedClass::~ThreadedClass()
{
	WaitForStop();
	// must call stop before desctruction
	assert(stop_thread);
}
