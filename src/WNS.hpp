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

#include <WNS/pyconfig/Parser.hpp>
#include <WNS/logger/Logger.hpp>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <list>
#include <vector>
#include <string>

namespace wns {

	/**
	 * @brief Handles the startup of the modules and different threads if networking
	 * is enabled
	 */
	class WNS
	{
	public:
		/**
		 * @brief Default Constructor
		 */
		WNS();
		/**
		 * @brief Default Destructor
		 */
		virtual
		~WNS();

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
		void init();

		/**
		 * @brief Run the simulation
		 */
		void run()
		{
		}

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

		std::string getPythonPath();

		static std::string prog_name;

		static std::string gdb_name;

		static bool attachGDB;

		static WNS* wns;

		static bool outputBT;

	private:


		static wns::logger::Logger log;

		std::string iniFileName;

		pyconfig::Parser pyConfig;

		bool be_verbose;

		bool testing;

		std::vector<std::string> testNames;

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

