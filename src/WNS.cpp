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

#include <WNS-CORE/WNS.hpp>
#include <WNS-CORE/OutputPreparation.hpp>
#include <WNS-CORE/SignalHandlers.hpp>

#include <WNS/logger/Master.hpp>
#include <WNS/Assure.hpp>
#include <WNS/module/Base.hpp>
#include <WNS/module/CurrentVersion.hpp>
#include <WNS/node/Node.hpp>
#include <WNS/node/Registry.hpp>
#include <WNS/events/scheduler/Interface.hpp>
#include <WNS/SmartPtrBase.hpp>
#include <WNS/probe/utils.hpp>
#include <WNS/TypeInfo.hpp>
#include <WNS/simulator/Simulator.hpp>

#include <WNS/rng/fibogen.hpp>
#include <WNS/speetcl/event.hpp>
#include <WNS/speetcl/pdu.hpp>
#include <WNS/speetcl/db.hpp>

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
	lazyBinding(false),
	absolute_path(false),
	be_verbose(false),
	testing(false),
	testNames(),
	mutex(),
	prematureAbort(false),
	options(),
	arguments()
{
	pthread_mutex_init(&mutex, NULL);

	options.add_options()

		("help,?",
		 "display this help and exit")

		("config-file,f",
		 boost::program_options::value<std::string>(&this->iniFileName)->default_value("config.py"),
		 "load config from configuration file")

		("lazy-linking,l",
		 boost::program_options::bool_switch(&this->lazyBinding),
		 "be lazy and link when needed (not at start-up)")

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


	if (NULL == )
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
	while(!registry.empty())
	{
		wns::node::Interface* doomed = registry.begin()->second;
		MESSAGE_SINGLE(NORMAL, log, "Deleting: " << doomed->getName());
		delete doomed;
		registry.erase(registry.begin()->first);
	}

	MESSAGE_SINGLE(NORMAL, log, "Destroying all modules");
	while(configuredModules.empty() == false)
	{
		wns::module::Base* mm = *(configuredModules.begin());
		MESSAGE_SINGLE(NORMAL, log, "Destroying module " << wns::TypeInfo::create(*mm));
		delete mm;
		configuredModules.erase(configuredModules.begin());
	}

    // shutdown ProbeBusRegistry
    if (probeBusRegistry != NULL)
    {
        delete probeBusRegistry;
    }

	wns->tearDownSPEETCL();

	std::cout << "\n"
		  << "Deleting EventScheduler" << std::endl;
	std::cout.flush();
 	wns::simulator::getSingleton().shutdownInstance();

#ifndef NDEBUG
	std::cout << "\n";
	std::cout << "PDUs available before Nodes deleted: " << pdusAvailableBeforeNodesDeletd << "\n";
	std::cout << "PDUs available after Nodes deleted:  " << PDU::getExistingPDUs() << "\n";
	std::cout << "Maximum number of PDUs available:    " << PDU::getMaxExistingPDUs() << "\n";
#ifdef WNS_SMARTPTR_DEBUGGING
	wns::SmartPtrBase::printAllExistingPointers();
#endif // WNS_SMARTPTR_DEBUGGING
#endif // NDEBUG


	if (this->pyConfig.get<bool>("WNS.postProcessing()") == false)
	{
		throw wns::Exception("WNS.postProcessing() failed!");
	}
	std::cout << "\n";
	std::cout << "Good bye. - ~WNS() finished" << std::endl;
}



void
WNS::abort()
{
	prematureAbort = true;
	wns::simulator::getEventScheduler()->queueCommand(SimControl::PrematureAbort(simctrl));
}

bool
WNS::isPrematureAbort()
{
	return prematureAbort;
}

void WNS::init()
{
	wns::simulator::getSingleton().setInstance(new wns::simulator::Simulator(pyConfig));
	signal(SIGSEGV, wns::catch_segv);
	signal(SIGABRT, wns::catch_abrt);
	signal(SIGINT,  wns::catch_int);
	signal(SIGUSR1, wns::catch_usr1);
	signal(SIGUSR2, wns::catch_usr2);
	signal(SIGXCPU, wns::catch_xcpu);

	if(this->networkingEnabled())
	{
		MESSAGE_BEGIN(NORMAL, log, m, "Start networking");
		MESSAGE_END();
		this->startNetworking();

		MESSAGE_BEGIN(NORMAL, log, m, "Binding network socket");
		MESSAGE_END();

		server->bind();

		MESSAGE_BEGIN(NORMAL, log, m, "Registering with GUI");
		MESSAGE_END();

		server->registerRemote();

		MESSAGE_BEGIN(NORMAL, log, m, "GUI found");
		MESSAGE_END();
	}

	pyconfig::View wnsView = pyConfig.getView("WNS");
	maxSimulationTime = pyConfig.get<double64>("WNS.maxSimTime");

	{ // prepare probes output directory according to the configured strategy

		pyconfig::View outputStrategyView = wnsView.getView("outputStrategy");

		std::auto_ptr<OutputPreparationStrategy> outputPreparationStrategy(
			OutputPreparationStrategyFactory::creator(
				outputStrategyView.get<std::string>("__plugin__"))->create());

		outputPreparationStrategy->prepare(wnsView.get<std::string>("outputDir"));
	}

	pyconfig::View masterLoggerView = wnsView.getView("masterLogger");
	outputBT = masterLoggerView.get<bool>("backtrace.enabled");

	log = logger::Logger(wnsView.get<wns::pyconfig::View>("logger"));
	// No simctrl needed in case of testing
	if (testing == false)
	{
		this->simctrl = new SimControl(this);
	}
	else
	{
		this->simctrl = NULL;
	}

	// Setup ProbeBus
    pyconfig::View pbregConfig = wnsView.getView("probeBusRegistry");
    probeBusRegistry = new wns::probe::bus::ProbeBusRegistry(pbregConfig);
    wns::simulator::getRegistry()->insert("WNS.ProbeBusRegistry",
                                          probeBusRegistry);

    wns::probe::bus::addProbeBusses(wnsView.get("probeBusses"));
}

void WNS::initSPEETCL()
{
	pyconfig::View pyConDataBase = pyConfig.getView("WNS.PDataBase");
	PDataBase* db = NULL;
	db = new PDataBase(pyConDataBase.get<uint32>("numBackups"),
			   pyConfig.get<String>("WNS.outputDir"),
			   pyConDataBase.get<bool>("compress"),
			   pyConDataBase.get<String>("zipCmd"),
			   pyConDataBase.get<String>("unzipCmd"),
			   pyConDataBase.get<String>("zipSuffix"),
			   pyConDataBase.get<double64>("settlingTime"),
			   pyConDataBase.get<double64>("PDBVersion"),
			   pyConDataBase.get<String>("prefix"),
			   pyConDataBase.get<bool>("storeInSQLiteDB"));

	wns::simulator::getRegistry()->insert("WNS.DataBase", db);
 	// Store PDataBase Pointer inside the PDataBase (!!scary!!)
 	PDataBase::setInstance(db);

	// read all probes which are globally registered
	wns::probe::registerProbesFromDict(pyConfig.getView("WNS.globalProbesDict"), db);

	if (maxSimulationTime > 0.0) {
		simctrl->setMaxSimTime(maxSimulationTime, pyConfig.get<bool>("WNS.fastShutdown"));
	}
}

void WNS::tearDownSPEETCL()
{
	wns::simulator::getRegistry()->erase("WNS.DataBase");
	std::cout << "Deleting PDataBase" << std::endl;
	std::cout.flush();
	PDataBase::destroy();
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


	if (this->arguments.count("show-modules"))
	{
		std::cout << "The following Modules are available before dynamic loading:" << std::endl;
		for(module::Factory::CreateMap::iterator i = module::Factory::getMap()->begin();
		    i != module::Factory::getMap()->end();
		    ++i)
		{
			std::cout << "\t"<< i->first << std::endl;
		}
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

	int32 nModules = pyConfig.len("WNS.modules");
	for(int i=0; i<nModules; ++i)
	{
		moduleViews.push_back(pyConfig.getView("WNS.modules", i));
	}
}

void WNS::shutdown()
{
	MESSAGE_SINGLE(NORMAL, log, "Calling onShutdown for all nodes");
	for(wns::node::Registry::const_iterator itr = registry.begin();
	    itr != registry.end();
	    ++itr) {
		itr->second->onShutdown();
	}

	MESSAGE_SINGLE(NORMAL, log, "Calling shutDown for all modules");
	for(std::list<module::Base*>::iterator itr = configuredModules.begin();
	    itr != configuredModules.end();
	    ++itr)
	{
		(*itr)->shutDown();
	}
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

std::string WNS::prog_name = "wns-core";
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
