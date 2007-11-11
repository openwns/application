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

#ifndef WNS_STATUSREPORT_HPP
#define WNS_STATUSREPORT_HPP

#include <WNS/events/PeriodicRealTimeout.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/probe/PutDecorator.hpp>

#include <ctime>
#include <string>

namespace wns
{
	class StatusReport :
		public wns::events::PeriodicRealTimeout
	{
	public:
		class WriteError :
			public wns::Exception
		{
		public:
			WriteError(const std::string& fileName) :
				wns::Exception("Couldn't write to file: " + fileName)
			{
			}
		};

		StatusReport();

		void
		start(const pyconfig::View& _pyConfigView);

		void
		stop();

		void
		periodically();

		void
		writeStatus(bool anEndOfSimFlag, std::string otherFileName = "");

	private:
		double maxSimTime;
		double settlingTime;
		time_t startTime;
		std::string outputDir;
		std::string statusFileName;
		std::string progressFileName;

		ProbeInterface* memoryConsumption;

		void
		probe();
	};
}

#endif // WNS_STATUSREPORT_HPP
