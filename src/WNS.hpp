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

#ifndef WNS_WNS_HPP
#define WNS_WNS_HPP

#include <WNS-CORE/StatusReport.hpp>

#include <WNS/pyconfig/Parser.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/node/Registry.hpp>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <list>
#include <vector>
#include <string>

namespace wns {
	class SimControl;

	/**
	 * @brief Handles the startup of the modules and different threads if networking
	 * is enabled
	 */
	class WNS
	{
	public:
		/**
		 * @brief Default Constructor */
		WNS();
		/**
		 * @brief Default Destructor
		 */
		virtual ~WNS();

		/**
		 * @brief Reads the command line parameters
		 *
		 * Returns "true" on success and "false" otherwise or if -h or -? has been
		 * specified
		 */
		void readCommandLineParameter(int argc, char* argv[]);

		/**
		 * @brief Configure stuff that belongs to WNS
		 */
		void initWNS();

		/**
		 * @brief Configure random number generator, event scheduler, ...
		 *
		 * SPEETCL is a library for dynamic, stochastic, event driven simulation and
		 * is configured by this method. E.g. the correct type of random number
		 * generator (Fibonacci, linear congruency) is selected.
		 */
		void initSPEETCL();

		/**
		 * @brief delete global objects created by speetcl
		 */
		void tearDownSPEETCL();


		/**
		 * @brief Write a fingerprint file to the output directory
		 *
		 */
		void writeFingerprint();

		/**
		 * @brief Start the simulation thread and (if enabled) the networking thread
		 */
		void run();

		/**
		 * @brief Stop Probes
		 */
		void stopProbes();

		/**
		 * @brief Shutdown all Module, Nodes, ...
		 */
		void shutdown();

		/**
		 * @brief Is testing enabled?
		 */
		bool testingEnabled()
		{
			return testing;
		}

		std::vector<std::string> getTestNames()
		{
			return testNames;
		}

		/**
		 * @brief Verbose?
		 */
		bool isVerbose()
		{
			return be_verbose;
		}

		/**
		 * @brief write the backtrace if configured
		 */
		void outputBacktrace();

		/**
		 * @brief triggers a premature abort of the simulation with
		 * the regular cleanup, probe writing, etc. wns-core will
		 * exit with status code 2
		 */
		void abort();

		std::string getPythonPath();

		/**
		 * @brief return premature abort status
		 *
		 * This is queried to determine wns-core's exit status.
		 *
		 * 0: Execution OK: maxSimTime reached
		 * 1: Error: Uncaught signal or exception
		 * 2: Aborted: Aborted during regular execution, because SIGUSR2
		 *    was received (e.g. sent by SGE upon resource limit violation)
		 */
		bool isPrematureAbort();

		static std::string prog_name;

		static std::string gdb_name;

		static bool attachGDB;

		static WNS* wns;

		static bool outputBT;

	private:

		simTimeType maxSimulationTime;


		static wns::logger::Logger log;

		std::string iniFileName;

		pyconfig::Parser pyConfig;

		StatusReport sr;

		SimControl* simctrl;

		bool lazyBinding;

		bool absolute_path;

		bool be_verbose;

		bool testing;

		std::vector<std::string> testNames;

		pthread_mutex_t mutex;

		/**
		 * @brief flag preserving the info whether the simulation was
		 * aborted prematurely
		 */
		bool prematureAbort;

		boost::program_options::options_description options;
		boost::program_options::variables_map arguments;
	};
}
#endif // NOT defined WNS_WNS_HPP

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

