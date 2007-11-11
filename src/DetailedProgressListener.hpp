/******************************************************************************
 * WNS (Wireless Network Simulator)                                          *
 * __________________________________________________________________________ *
 *                                                                            *
 * Copyright (C) 2004-2005                                                    *
 * Chair of Communication Networks (ComNets)                                  *
 * Kopernikusstr. 16, D-52074 Aachen, Germany                                 *
 * phone: ++49-241-80-27910 (phone), fax: ++49-241-80-22242                   *
 * email: wns@comnets.rwth-aachen.de                                          *
 * www: http://wns.comnets.rwth-aachen.de                                     *
 ******************************************************************************/

#ifndef WNSCORE_DETAILEDPROGRESSLISTENER_HPP
#define WNSCORE_DETAILEDPROGRESSLISTENER_HPP

#include <cppunit/TextTestProgressListener.h>
#include <cppunit/Test.h>

class DetailedProgressListener :
	public CppUnit::TextTestProgressListener
{

public:
	void
	startTest(CppUnit::Test* test);

	void
	endTest(CppUnit::Test* test);

	void
	addFailure(const CppUnit::TestFailure& failure);

private:
	bool failed;

};

#endif // NOT defined WNSCORE_DETAILEDPROGRESSLISTENER_HPP
