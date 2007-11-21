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
#include <OPENWNS/bversion.hpp>

#include <WNS/testing/DetailedListener.hpp>
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
    status_(0),
    configFile_("config.py"),
    configuration_(),
    verbose_(false),
    testing_(false),
    testNames_(),
    pyConfigPatches_(),
    options_(),
    arguments_(),
    programName_("openwns"),
    debuggerName_("gdb"),
    attachDebugger_(false),
    signalHandler_(),
    interactiveConfig_()
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

        ("interactive-configuration,i",
         boost::program_options::bool_switch(&interactiveConfig_),
         "after reading config start an interactive shell which allows modification of the configuration, use 'continue' to exit shell and run openWNS")

        ("unit-tests,t",
         boost::program_options::bool_switch(&testing_),
         "test mode: run unit tests specified with -T or default suite if no tests with -T given")

        ("named-unit-tests,T",
         boost::program_options::value<TestNameContainer>(&testNames_),
         "run named unit test (defined multipple time for multiple tests), e.g. wns::pyconfig::tests::ParserTest, use with -t")

        ("python-path,P",
         "print Python path and exit")

        ("verbose,v",
         boost::program_options::bool_switch(&verbose_),
         "verbose mode (version information and verbose tests)")

        ("patch-config,y",
         boost::program_options::value<PyConfigPatchContainer>(&pyConfigPatches_),
         "patch the configuration with the given Python expression")
        ;
}

WNS::~WNS() throw()
{
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
        status_ = 0;
        exit(0);
    }

    if (arguments_.count("python-path") > 0)
    {
        std::cout << getPythonPath() << std::endl;
        status_ = 0;
        exit(0);
    }

    // if -s is specified we need to attach the debugger
    attachDebugger_ = arguments_.count("attach-debugger-on-segfault") > 0;

    // patch pyconfig (sys.path, command line patches ...)
    configuration_.appendPath(getPythonPath());
    configuration_.appendPath(".");
    configuration_.load(configFile_);
    for(PyConfigPatchContainer::const_iterator it = pyConfigPatches_.begin();
        it != pyConfigPatches_.end();
        ++it)
    {
        configuration_.patch(*it);
    }

    if(interactiveConfig_)
    {
        configuration_.patch("import pdb; pdb.set_trace()");
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

        // setup testrunner
        CppUnit::TextTestRunner runner;
        std::auto_ptr<CppUnit::TestListener> listener;
        if(verbose_)
        {
            listener.reset(new wns::testing::DetailedListener());
        }
        else
        {
            listener.reset(new CppUnit::TextTestProgressListener());
        }
        runner.eventManager().addListener(listener.get());
        runner.addTest(masterSuite);

        // Built tests (either all, or only specific ones given on the
        // command line)
        if(!testNames_.empty())
        {

            for(TestNameContainer::const_iterator ii = testNames_.begin();
                ii != testNames_.end();
                ++ii)
            {
                status_ = runner.run(*ii, false, true, false) ? status_ : 1;
            }
        }
        else
        {
            status_ = runner.run("DefaultTests", false, true, false) ? 0 : 1;
        }

    }
    // normal simulation
    else
    {
        status_ = 0;
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
    std::cerr << "openWNS: caught an unexpected excpetion!\n";
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
    // block all signals until we have removed the handler
    sigset_t allSignals;
    sigfillset(&allSignals);
    sigprocmask(SIG_BLOCK, &allSignals, NULL);
    if (!map_.knows(signum))
    {
        // should never happen, otherwise the implementation is broken
        std::cerr << "openWNS: Tried to removed signal handler for signal " << signum <<"!!!\n";
        std::cerr << "         But no handler was registered.";
    }
    else
    {
        map_.erase(signum);
        // restore default signal handler
        struct sigaction action;
        sigfillset (&action.sa_mask);
        action.sa_flags = 0;
        action.sa_handler = SIG_DFL;
        sigaction(signum, &action, NULL);
    }
    sigprocmask(SIG_UNBLOCK, &allSignals, NULL);
}

void
WNS::SignalHandler::removeAllSignalHandlers()
{
    // block all signals until we have removed all handlers
    sigset_t allSignals;
    sigfillset(&allSignals);
    sigprocmask(SIG_BLOCK, &allSignals, NULL);
    while(!map_.empty())
    {
        Map::const_iterator itr = map_.begin();
        // restore default signal handler
        struct sigaction action;
        sigfillset (&action.sa_mask);
        action.sa_flags = 0;
        action.sa_handler = SIG_DFL;
        sigaction(itr->first, &action, NULL);
        map_.erase(itr->first);
    }
    sigprocmask(SIG_UNBLOCK, &allSignals, NULL);
}

void
WNS::SignalHandler::catchSignal(int signum)
{
    std::cerr << "\nopenWNS: caught signal " << signum << "\n";
    wns::WNS& wns = wns::WNSSingleton::Instance();
    if (!wns.signalHandler_.map_.knows(signum))
    {
        // should never happen, otherwise the implementation is broken
        std::cerr << "openWNS: no signal handler defined!!!\n";
        return;
    }

    Handler* handler = wns.signalHandler_.map_.find(signum);

    if (handler->num_slots() == 0)
    {
        std::cerr << "openWNS: no signal handler to call!\n";
        return;
    }
    if (handler->num_slots() > 1)
    {
        std::cerr << "openWNS: more than one signal handler to call! Not calling any signal handler!\n";
        return;
    }

    // call the signal handler
    (*handler)();
}
