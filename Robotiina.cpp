// Robotiina.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "robot.h"

int main(int argc, char *argv[])
{
	try
	{
		Robot robotiina;
		return robotiina.Launch(argc, argv);
	}
	catch (std::exception &e)
	{
		std::cout << "ups, " << e.what() << std::endl;
	}
	catch (const std::string &e)
	{
		std::cout << "ups, " << e << std::endl;
	}
	catch (...)
	{
		std::cout << "ups, did not see that coming."<< std::endl;
	}

}
