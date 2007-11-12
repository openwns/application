/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2007
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 16, D-52074 Aachen, Germany
 * phone: ++49-241-80-27910,
 * fax: ++49-241-80-22242
 * email: info@openwns.org
 * www: http://www.openwns.org
 * _____________________________________________________________________________
 *
 * openWNS is free software; you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License version 2 as published by the
 * Free Software Foundation;
 *
 * openWNS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include <OPENWNS/WNS.hpp>
#include <OPENWNS/SignalHandlers.hpp>
#include <OPENWNS/DetailedProgressListener.hpp>
#include <OPENWNS/bversion.hpp>

#include <WNS/logger/Master.hpp>
#include <WNS/Assure.hpp>
#include <WNS/TypeInfo.hpp>
#include <WNS/TestFixture.hpp>
#include <WNS/events/scheduler/Interface.hpp>
#include <WNS/simulator/Simulator.hpp>
#include <WNS/simulator/UnitTests.hpp>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>

#include <boost/program_options/value_semantic.hpp>

#include <csignal>
#include <dlfcn.h>
#include <iomanip>
#include <memory>
#include <fstream>

using namespace wns;

WNS::WNS() :
        configFile_("config.py"),
        configuration_(),
        verbose_(false),
        testing_(false),
        testNames_(),
        options_(),
        arguments_(),
        programName_("openwns"),
        debuggerName_("gdb"),
        attachDebugger_(false),
	signalHandler_()
{
        options_.add_options()

                ("help,?",
                 "display this help and exit")

                ("config-file,f",
                 boost::program_options::value<std::string>(&configFile_)->default_value("config.py"),
                 "load config from configuration file")

                ("attach-debugger-on-segfault,s",
                 boost::program_options::value<std::string>(&debuggerName_),
                 "fire up gdb on segfault, arg = command for debugger")

                ("stop-in-debugger-on-assure,d",
                 boost::program_options::bool_switch(&wns::Assure::useSIGTRAP),
                 "stop in debugger if an 'assure' fired (no exception will be thrown)")

                ("unit-tests,t",
                 boost::program_options::bool_switch(&testing_),
                 "test mode: run unit tests specified with -T or default suite if no tests with -T given")

                ("named-unit-tests,T",
                 boost::program_options::value< std::vector<std::string> >(&testNames_),
                 "run named unit test (defined multipple time for multiple tests), e.g. wns::pyconfig::tests::ParserTest, use with -t")

                ("python-path,P",
                 "print Python path and exit")

                ("verbose,v",
                 boost::program_options::bool_switch(&verbose_),
                 "verbose mode (version information and verbose tests)")

                ("patch-config,y",
                 boost::program_options::value<std::string>(),
                 "patch the configuration with the given Python expression")
                ;
}

WNS::~WNS() throw()
{
        // reset signal handlers to default
        signal(SIGSEGV, SIG_DFL);
        signal(SIGABRT, SIG_DFL);
        signal(SIGINT,  SIG_DFL);
        signal(SIGUSR1, SIG_DFL);

        std::cout << "\nopenWNS terminated\n";
}

void
WNS::readCommandLine(int argc, char* argv[])
{
	programName_ = std::string(argv[0]);

        boost::program_options::store(
                boost::program_options::parse_command_line(argc, argv, options_),
		arguments_);
        boost::program_options::notify(arguments_);
}

void WNS::init()
{
        if (arguments_.count("help") > 0)
        {
                std::cout << options_ << "\n";
                exit(1);
        }

        if (arguments_.count("python-path") > 0)
        {
                std::cout << getPythonPath() << std::endl;
                exit(0);
        }

        attachDebugger_ = arguments_.count("attach-debugger-on-segfault") > 0;

	// patch pyconfig (sys.path, command line patches ...)
        typedef std::list<std::string> PyConfigPatches;
        PyConfigPatches pyConfigPatches;

        if (arguments_.count("patch-config") > 0)
        {
                pyConfigPatches.push_back(arguments_["patch-config"].as<std::string>());
        }


        configuration_.appendPath(getPythonPath());
        configuration_.appendPath(".");
        configuration_.load(configFile_);
        for(PyConfigPatches::iterator it = pyConfigPatches.begin();
            it != pyConfigPatches.end();
            ++it)
        {
                configuration_.patch(*it);
        }

	// after pyconfig is patched, bring up Simulator singelton
        if (testing_)
        {
                wns::simulator::getSingleton().setInstance(new wns::simulator::UnitTests(configuration_));
        }
        else
        {
                wns::simulator::getSingleton().setInstance(new wns::simulator::Simulator(configuration_));
        }

	// after this we can install the signal handlers as well as the
	// unexpected handlers, because both need the SimulatorSingleton

	// for the backtrace to work we need to set out own unexpected handler
	// for exceptions
	std::set_unexpected(wns::WNS::unexpectedHandler);

	// register signal handlers

	signalHandler_.addSignalHandler(
		SIGABRT,
		wns::signalhandler::Abort());

	signalHandler_.addSignalHandler(
		SIGSEGV,
		wns::signalhandler::SegmentationViolation(
			attachDebugger_,
			debuggerName_,
			programName_));

	signalHandler_.addSignalHandler(
		SIGINT,
		wns::signalhandler::Interrupt());

	signalHandler_.addSignalHandler(
		SIGUSR1,
		wns::signalhandler::UserDefined1());
}

void
WNS::run()
{
        // Unit tests are processed here
        if(testing_)
        {
                CppUnit::TestFactoryRegistry& defaultRegistry = CppUnit::TestFactoryRegistry::getRegistry(wns::testsuite::Default());
                CppUnit::TestSuite* masterSuite = new CppUnit::TestSuite("AllTests");
                CppUnit::TestSuite* defaultSuite = new CppUnit::TestSuite("DefaultTests");
                defaultSuite->addTest(defaultRegistry.makeTest());
                defaultSuite->addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
                masterSuite->addTest(defaultSuite);

                // run the specified tests
                bool wasSuccessful = false;
                CppUnit::TextTestRunner runner;
                // Built tests (either all, or only specific ones given on the
                // command line)
                if(!testNames_.empty())
                {
                        // register disabled tests
                        CppUnit::TestFactoryRegistry& disabledRegistry =
                                CppUnit::TestFactoryRegistry::getRegistry(wns::testsuite::Disabled());
                        masterSuite->addTest(disabledRegistry.makeTest());

                        // register performance tests
                        CppUnit::TestFactoryRegistry& performanceRegistry =
                                CppUnit::TestFactoryRegistry::getRegistry(wns::testsuite::Performance());
                        masterSuite->addTest(performanceRegistry.makeTest());

                        // register spikes
                        CppUnit::TestFactoryRegistry& spikeRegistry =
                                CppUnit::TestFactoryRegistry::getRegistry(wns::testsuite::Spike());
                        masterSuite->addTest(spikeRegistry.makeTest());

                        for(std::vector<std::string>::iterator ii = testNames_.begin();
                            ii != testNames_.end();
                            ++ii)
                        {
                                runner.addTest(masterSuite->findTest(*ii));
                        }
                }
                else
                {
                        runner.addTest(masterSuite);
                }

                if(verbose_)
                {
                        DetailedProgressListener progress;
                        runner.eventManager().addListener(&progress);
                        wasSuccessful = runner.run("", false, true, false);
                }
                else
                {
                        wasSuccessful = runner.run("", false);
                }
        }

}

void WNS::shutdown()
{
        if (!configuration_.get<bool>("WNS.postProcessing()"))
        {
                throw wns::Exception("WNS.postProcessing() failed!");
        }

	// deregister all signal handler before shutting down master logger
	signalHandler_.removeAllSignalHandlers();

	// restore default state
	std::set_unexpected(std::terminate);

        // This is the very last thing to shut down! Keep the MasterLogger up as
        // long as possible
        wns::simulator::getSingleton().shutdownInstance();
}


std::string
WNS::getPythonPath() const
{
        std::stringstream ss;
        // shouldn't this be done via wnsrc.py?
        ss << INSTALLPREFIX << "/" << BUILDFLAVOUR << "/lib" << "/PyConfig";
        return ss.str();
}

int
WNS::status() const
{
        return status_;
}

void
WNS::unexpectedHandler()
{
	std::cout << "openWNS: caught an unexpected excpetion!\n";
	wns::simulator::getMasterLogger()->outputBacktrace();
	exit(1);
}


// SignalHandler implementation ...

WNS::SignalHandler::SignalHandler() :
	map_()
{
}

WNS::SignalHandler::~SignalHandler()
{
	removeAllSignalHandlers();
}

void
WNS::SignalHandler::removeSignalHandler(int signum)
{
	if (!map_.knows(signum))
	{
		// should never happen, otherwise the implementation is broken
		std::cout << "openWNS: Tried to removed signal handler for signal " << signum <<"!!!\n";
		std::cout << "         But no handler was registered.";
		return;
	}
	map_.erase(signum);
	// restore default signal handler
	signal(signum, SIG_DFL);
}

void
WNS::SignalHandler::removeAllSignalHandlers()
{
	while(!map_.empty())
	{
		Map::const_iterator itr = map_.begin();
		// restore old signal handler
		signal(itr->first, SIG_DFL);
		// remove signal handler from map
		map_.erase(itr->first);
	}
}

void
WNS::SignalHandler::catchSignal(int signum)
{
	std::cout << "openWNS: caught signal " << signum << "\n";
	wns::WNS& wns = wns::WNSSingleton::Instance();
	if (!wns.signalHandler_.map_.knows(signum))
	{
		// should never happen, otherwise the implementation is broken
		std::cout << "openWNS: no signal handler defined!!!\n";
		return;
	}

	Handler* handler = wns.signalHandler_.map_.find(signum);

	if (handler->num_slots() == 0)
	{
		std::cout << "openWNS: no signal handler to call!\n";
		return;
	}
	if (handler->num_slots() > 1)
	{
		std::cout << "openWNS: more than one signal handler to call! Not calling any signal handler!\n";
		return;
	}
	// call the signal handler
	(*handler)();
}
