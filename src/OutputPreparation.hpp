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

#ifndef WNSCORE_OUTPUTPREPARATION_HPP
#define WNSCORE_OUTPUTPREPARATION_HPP

#include <WNS/StaticFactory.hpp>

#include <string>

namespace wns {

	class OutputPreparationStrategy
	{
	public:
		virtual
		~OutputPreparationStrategy()
		{}

		virtual void
		prepare(const std::string& path) = 0;
	};

	typedef wns::Creator<OutputPreparationStrategy> OutputPreparationStrategyCreator;
	typedef wns::StaticFactory<OutputPreparationStrategyCreator> OutputPreparationStrategyFactory;

	class Move :
		public OutputPreparationStrategy
	{
	public:
		virtual void
		prepare(const std::string& path);
	};

	class Delete :
		public OutputPreparationStrategy
	{
	public:
		virtual void
		prepare(const std::string& path);
	};

} // wns

#endif // NOT defined WNSCORE_OUTPUTPREPARATION_HPP

/*
  Local Variables:
  mode: c++
  fill-column: 80
  c-basic-offset: 8
  c-tab-always-indent: t
  indent-tabs-mode: t
  tab-width: 8
  End:
*/

