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

#ifndef WNS_SIMCONTROL_HPP
#define WNS_SIMCONTROL_HPP

#include <WNS/speetcl/event.hpp>
#include <WNS/events/PeriodicTimeout.hpp>

#include <WNS/logger/Logger.hpp>

namespace wns
{
	class WNS;
	class SimControl :
		public events::PeriodicTimeout
	{
		/**
		 * @brief Responsible for Shutdown of simulation
		 */
		class Shutdown :
			public Event
		{
		public:
			explicit
			Shutdown(SimControl* _simControl);

			virtual void
			execute();

		private:
			SimControl* simControl;
		};

	public:
		/**
		 * @brief Responsible for Premature Shutdown of simulation
		 *
		 * This is derived from command instead of "Event", because it
		 * needs to be "injected" into the already running scheduler thread.
		 * When executed, this command schedules a regular Shutdown event for
		 * the current point in Simtime. 
		 */
		class PrematureAbort
		{
			SimControl* simControl;
		public:
			explicit
			PrematureAbort(SimControl* _simControl);

			void
			operator()();
		};


		explicit
		SimControl(WNS* _wns);

		virtual
		~SimControl();

		void
		setMaxSimTime(simTimeType anEndTime, bool fastShutdown);

		/**
		 * @brief Call this to end the simulation NOW. Everything will behave
		 * as if maxSimTime had been reached.
		 */

		virtual void
		periodically();

	private:
		WNS* wns;
		logger::Logger log;
		bool fastShutdown;
	};
}

#endif // NOT defined WNS_SIMCONTROL_HPP

/*
  Local Variables:
  mode: c++
  fill-column: 80
  c-basic-offset: 4
  c-tab-always-indent: t
  indent-tabs-mode: t
  tab-width: 4
  End:

*/
