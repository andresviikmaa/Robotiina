#pragma once
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>

class ThreadedClass
{
public:
	ThreadedClass(); 
	void Start();
	void Stop();
	void WaitForStop() { Stop(); }
	bool HasError() { return stop_thread; };

	~ThreadedClass();
protected:
	boost::thread_group threads;
	boost::atomic<bool> stop_thread;
	virtual void Run() = 0;

};

