/******************************************************************************
 * WNS (Wireless Network Simulator)                                           *
 * __________________________________________________________________________ *
 *                                                                            *
 * Copyright (C) 2004-2006                                                    *
 * Chair of Communication Networks (ComNets)                                  *
 * Kopernikusstr. 16, D-52074 Aachen, Germany                                 *
 * phone: ++49-241-80-27910 (phone), fax: ++49-241-80-22242                   *
 * email: wns@comnets.rwth-aachen.de                                          *
 * www: http://wns.comnets.rwth-aachen.de                                     *
 ******************************************************************************/

#include <OPENWNS/DetailedProgressListener.hpp>

#include <iostream>


void
DetailedProgressListener::startTest(CppUnit::Test* test)
{
	failed = false;
	std::cout << "\033[01;35m[TST]\033[00m " << test->getName();
}

void
DetailedProgressListener::addFailure(const CppUnit::TestFailure&)
{
	failed = true;
}

void
DetailedProgressListener::endTest(CppUnit::Test* test)
{
	int length = test->getName().length();
	std::string padding = "";
	if (length < 90) {
		padding.append(90 - length, ' ');
	}

	if (failed) {
		std::cout << padding << "\033[01;31m[FAILED]\033[00m" << std::endl;
	}
	else {
		std::cout << padding << "\033[01;32m[OK]\033[00m" << std::endl;
	}

}

