
#include "ThreadedClass.h"


ThreadedClass::ThreadedClass()
{
	stop_thread = false;
}

void ThreadedClass::Start()
{
	threads.create_thread(boost::bind(&ThreadedClass::Run, this));
};
void ThreadedClass::Stop()
{
	stop_thread = true;
	threads.join_all();
};

ThreadedClass::~ThreadedClass()
{
	// must call stop before desctruction
	assert(stop_thread);
}
