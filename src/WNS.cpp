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

#include <OPENWNS/WNS.hpp>
#include <OPENWNS/SignalHandlers.hpp>
#include <OPENWNS/bversion.hpp>

#include <WNS/logger/Master.hpp>
#include <WNS/Assure.hpp>
#include <WNS/events/scheduler/Interface.hpp>
#include <WNS/TypeInfo.hpp>
#include <WNS/simulator/Simulator.hpp>

#include <boost/program_options/value_semantic.hpp>

#include <csignal>
#include <dlfcn.h>
#include <iomanip>
#include <memory>
#include <fstream>

using namespace wns;

WNS::WNS() :
        iniFileName("config.py"),
        pyConfig(),
	be_verbose(false),
	testing(false),
	testNames(),
	options(),
	arguments()
{
	options.add_options()

		("help,?",
		 "display this help and exit")

		("config-file,f",
		 boost::program_options::value<std::string>(&this->iniFileName)->default_value("config.py"),
		 "load config from configuration file")

		("attach-debugger-on-segfault,s",
		 boost::program_options::value<std::string>(&WNS::gdb_name),
		 "fire up gdb on segfault, arg = command for debugger")

		("stop-in-debugger-on-assure,d",
		 boost::program_options::bool_switch(&wns::Assure::useSIGTRAP),
		 "stop in debugger if an 'assure' fired (no exception will be thrown)")

		("unit-tests,t",
		 boost::program_options::bool_switch(&this->testing),
		 "run all specified and loaded unit tests")

		("named-unit-tests,T",
		 boost::program_options::value< std::vector<std::string> >(&this->testNames),
		 "run named unit test (defined multipple time for multiple tests), e.g. wns::pyconfig::tests::ParserTest")

		("python-path,P",
		 "print Python path and exit")

		("verbose,v",
		 boost::program_options::bool_switch(&this->be_verbose),
		 "verbose mode (version information and verbose tests)")

		("patch-config,y",
		 boost::program_options::value<std::string>(),
		 "patch the configuration with the given Python expression")
		;


	if (NULL == wns)
	{
		wns = this;
	}
	else
	{
		std::cout << "Only one instance of WNS allowed!" << std::endl;
		exit(1);
	}
}

WNS::~WNS()
{
	MESSAGE_SINGLE(NORMAL, log, "Deleting all nodes");

	MESSAGE_SINGLE(NORMAL, log, "Destroying all modules");

	std::cout << "\n"
		  << "Deleting EventScheduler" << std::endl;
	std::cout.flush();
 	wns::simulator::getSingleton().shutdownInstance();

	if (this->pyConfig.get<bool>("WNS.postProcessing()") == false)
	{
		throw wns::Exception("WNS.postProcessing() failed!");
	}
	std::cout << "\n";
	std::cout << "Good bye. - ~WNS() finished" << std::endl;
}

void WNS::init()
{
	wns::simulator::getSingleton().setInstance(new wns::simulator::Simulator(pyConfig));
	signal(SIGSEGV, wns::catch_segv);
	signal(SIGABRT, wns::catch_abrt);
	signal(SIGINT,  wns::catch_int);
	signal(SIGUSR1, wns::catch_usr1);

	pyconfig::View wnsView = pyConfig.getView("WNS");

	pyconfig::View masterLoggerView = wnsView.getView("masterLogger");
	outputBT = masterLoggerView.get<bool>("backtrace.enabled");

	log = logger::Logger(wnsView.get<wns::pyconfig::View>("logger"));
}

void
WNS::readCommandLineParameter(int argc, char* argv[])
{
	boost::program_options::store(
	    boost::program_options::parse_command_line(
	        argc,argv,this->options),
	    this->arguments);
	boost::program_options::notify(this->arguments);

	if (this->arguments.count("help"))
	{
		std::cout << this->options << "\n";
		exit(1);
	}

	if (this->arguments.count("python-path"))
	{
		std::cout << getPythonPath() << std::endl;
		exit(0);
	}

	WNS::attachGDB = this->arguments.count("attach-debugger-on-segfault");

	if (this->arguments.count("named-unit-tests"))
	{
		this->testing = true;
	}

	typedef std::list<std::string> PyConfigPatches;
	PyConfigPatches pyConfigPatches;

	if (this->arguments.count("patch-config"))
	{
		pyConfigPatches.push_back(this->arguments["patch-config"].as<std::string>());
	}


	pyConfig.appendPath(getPythonPath());
	pyConfig.appendPath(".");
	pyConfig.load(iniFileName);
	for(PyConfigPatches::iterator it = pyConfigPatches.begin();
	    it != pyConfigPatches.end();
	    ++it)
	{
		pyConfig.patch(*it);
	}
}

void WNS::shutdown()
{
}


void WNS::outputBacktrace()
{
	if (WNS::outputBT)
	{
		wns::simulator::getMasterLogger()->outputBacktrace();
	}
}

std::string WNS::getPythonPath()
{
	std::stringstream ss;
	// shouldn't this be done via wnsrc.py?
	ss << INSTALLPREFIX << "/" << BUILDFLAVOUR << "/lib" << "/PyConfig";
	return ss.str();
}

std::string WNS::prog_name = "openwns";
std::string WNS::gdb_name = "gdb";
bool WNS::attachGDB = false;
WNS* WNS::wns = 0;
wns::logger::Logger WNS::log = logger::Logger("WNS", "WNS", NULL);
bool WNS::outputBT = false;

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
