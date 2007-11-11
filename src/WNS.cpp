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
#include <WNS-CORE/SimControl.hpp>
#include <WNS-CORE/server/server.h>
#include <WNS-CORE/bversion.hpp>
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

VersionInformation theWNSCoreVersionInformation = wns::module::VersionInformation(BUILDVINFO);


WNS::WNS() :
	maxSimulationTime(0.0),
	stepCounter(0),
	networking(false),
	stepping(false),
	listLoadedModules(false),
	port(0),
	configuredModules(std::list<module::Base*>(0)),
        iniFileName("config.py"),
        pyConfig(),
	probeBusRegistry(NULL),
	sr(),
	simctrl(NULL),
	commandLineModules(),
	lazyBinding(false),
	absolute_path(false),
	be_verbose(false),
	testing(false),
	testingStyle("text"),
	testNames(),
	readLibsFromCommandLine(false),
	registry(),
	mutex(),
	prematureAbort(false),
	options(),
	arguments()
{
	pthread_mutex_init(&mutex, NULL);

	options.add_options()

		("help,?",
		 "display this help and exit")

		("load-modules,m",
		 boost::program_options::value< std::vector<std::string> >(&this->commandLineModules),
		 "load modules as specified here (to load multiple modules, specify multiple times: -m foo -m bar)")

		("show-modules,M",
		 "show modules that have been loaded")

		("config-file,f",
		 boost::program_options::value<std::string>(&this->iniFileName)->default_value("config.py"),
		 "load config from configuration file")

		("lazy-linking,l",
		 boost::program_options::bool_switch(&this->lazyBinding),
		 "be lazy and link when needed (not at start-up)")

		("networking,n",
		 boost::program_options::bool_switch(&this->networking),
		 "enables networking [EXPERIMENTAL]")

		("attach-debugger-on-segfault,s",
		 boost::program_options::value<std::string>(&WNS::gdb_name),
		 "fire up gdb on segfault, arg = command for debugger")

		("stop-in-debugger-on-assure,d",
		 boost::program_options::bool_switch(&wns::Assure::useSIGTRAP),
		 "stop in debugger if an 'assure' fired (no exception will be thrown)")

		("unit-tests,t",
		 boost::program_options::bool_switch(&this->testing),
		 "run all specified and loaded unit tests")

		("test-ui-style,k",
		 boost::program_options::value<std::string>(&this->testingStyle)->default_value("text"),
		 "style = [text|curses|qt] (use in combination with -t)")

		("named-unit-tests,T",
		 boost::program_options::value< std::vector<std::string> >(&this->testNames),
		 "run named unit test (defined multipple time for multiple tests), e.g. wns::pyconfig::tests::ParserTest")

		("port,p",
		 boost::program_options::value<int>(&this->port)->default_value(0),
		 "port for networking socket")

		("python-path,P",
		 "print Python path and exit")

		("verbose,v",
		 boost::program_options::bool_switch(&this->be_verbose),
		 "verbose mode (version information and verbose tests)")

		("patch-config,y",
		 boost::program_options::value<std::string>(),
		 "patch the configuration with the given Python expression")

		("single-step-mode,w",
		 boost::program_options::bool_switch(&this->stepping),
		 "wait after event. Single-step mode")
		;


	if (!wns) {
		wns=this;
	} else {
		std::cout << "Only one instance of WNS allowed!" << std::endl;
		exit(1);
	}
}

WNS::~WNS()
{
#ifndef NDEBUG
	int32 pdusAvailableBeforeNodesDeletd = PDU::getExistingPDUs();
#endif
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

	// shutdown SimControl
	if (simctrl != NULL)
	{
		delete simctrl;
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


bool WNS::loadModules()
{
	std::list<VersionInformation> moduleVersions;

	if(be_verbose) {
		std::cout << theWNSCoreVersionInformation.getNiceString() << std::endl;
		std::cout << wns::module::CurrentVersion.getNiceString() << std::endl;
		std::cout << "Loading..." << std::endl;
	}

	moduleVersions.push_back(theWNSCoreVersionInformation);
	moduleVersions.push_back(wns::module::CurrentVersion);

	// If Modules should be read from command line, we will remove all modules
	// specified in the config file, that haven't been specified on the command
	// line. This means only Modules specified in the config file can be loaded
	// on the command line.

	// (msg) There must be an easier way to do this!! However, it's working ...
	std::list<pyconfig::View> moduleViewsTmp = moduleViews;
	if(readLibsFromCommandLine) {
		moduleViews.clear();
		for(std::list<pyconfig::View>::iterator i = moduleViewsTmp.begin();
		    i != moduleViewsTmp.end();
		    ++i) {
			std::vector<std::string>::iterator i2;
			for(i2 = commandLineModules.begin();
			    i2 != commandLineModules.end();
			    ++i2) {
				if (i->get<std::string>("libname") == *i2) {
					moduleViews.push_back(*i);
					break;
				}
			}
		}
	}

	if (moduleViews.empty()){
		std::cerr << "Please load at least one simulator library. " << std::endl;
		std::cerr << "Exiting..." << std::endl;
		return false;
	}

	moduleViewsTmp = moduleViews;

	std::list<pyconfig::View>::iterator itr = moduleViewsTmp.begin();
	uint32 numberOfErrors = 0;
	std::string errorStr;

	// walk through the list, every time a module can be successfully loaded it
	// is removed from the list and we start from the beginning of the list.
	// This will be done until the list is empty non of the remaining modules
	// can be loaded.It can happen that module can't be loaded the first
	// time. This is because the module might have dependencies on other
	// modules which are not loaded yet.
	while(!moduleViewsTmp.empty()) {
		if(numberOfErrors == moduleViewsTmp.size()) {
			std::cout << "ModuleFactory contains the following modules:" << std::endl;
			for(module::Factory::CreateMap::iterator i = module::Factory::getMap()->begin();
			    i != module::Factory::getMap()->end();
			    ++i) {
				std::cout << "\t"<< i->first << std::endl;
			}
			throw wns::Exception(
				"Can't load all specified modules. Reason:\n" +
				errorStr);
		}

		std::string libName = itr->get<std::string>("libname");
		std::string moduleName = itr->get<std::string>("__plugin__");

		if(be_verbose) {
			std::cout << std::setw(8) << "Library: " << libName << "\n"
				  << "Module: " << moduleName << "\n";
		}
		// If the ModuleFactory knows the Module the library has been opened
		// before (may be due to static linkage). Then we don't need to load the
		// library by hand.
		if (module::Factory::knows(moduleName) == false)
		{
			bool success = wns::module::Base::load(libName, absolute_path, be_verbose, lazyBinding);
			if(success == false)
			{
				// continue with next candidate
				errorStr += std::string(dlerror());
				numberOfErrors += 1;
				++itr;
				continue;
			}
		}
		// it seems we found a loadable module -> append to module list
		module::Creator* moduleCreator = module::Factory::creator(moduleName);
		module::Base* m = moduleCreator->create(*itr);
		configuredModules.push_back(m);
		VersionInformation v = m->getVersionInformation();
		moduleVersions.push_back(v);
		if(be_verbose)
		{
			std::cout << v.getNiceString() << std::endl;
			std::cout << std::endl;
		}
		moduleViewsTmp.erase(itr);
		// After a loadable module has been found and an error happened
		// before (another module could not be loaded), the first module
		// in the list is the one that could not be loaded. Thus we put
		// it to the end.
		if(numberOfErrors > 0)
		{
			moduleViewsTmp.push_back((*moduleViewsTmp.begin()));
			moduleViewsTmp.erase(moduleViewsTmp.begin());
		}
		itr = moduleViewsTmp.begin();
		numberOfErrors = 0;
	}

	if(listLoadedModules) {
		std::cout << "The following Modules are available after dynamic loading:" << std::endl;
		for(module::Factory::CreateMap::iterator i = module::Factory::getMap()->begin();
		    i != module::Factory::getMap()->end();
		    ++i) {
			std::cout << "\t"<< i->first << std::endl;
		}
	}

	if(!checkModuleDependencies(moduleVersions)) {
		return false;
	}

	return true;
}

void WNS::writeFingerprint()
{
	std::string fingerprintName = pyConfig.get<std::string>("WNS.outputDir") + "/WNSFingerprint.dat";
	std::ofstream fingerprint(fingerprintName.c_str());
	if(!fingerprint.good())
		throw GenErr("Couldn't create fingerprint file.");

	fingerprint
		<< theWNSCoreVersionInformation.getString() << std::endl
		<< wns::module::CurrentVersion.getString() << std::endl
		;

	for(std::list<module::Base*>::iterator it = configuredModules.begin();
		it != configuredModules.end();
		++it) {
		fingerprint << (*it)->getVersionInformation().getString() << std::endl;
	}

	fingerprint.close();
}

bool WNS::checkModuleDependencies(std::list<VersionInformation> moduleVersions)
{
	std::list<VersionInformation>::iterator itrMV;
	std::list<VersionInformation>::iterator itrMVEnd = moduleVersions.end();

	for (itrMV = moduleVersions.begin(); itrMV!=itrMVEnd; ++itrMV) {
		if (!(*itrMV).getDependencies().dependenciesMetBy(moduleVersions.begin(),
								  moduleVersions.end())) {
			std::cerr << "ERROR: Dependencies not met:" << std::endl;
			std::cerr << (*itrMV).getNiceString();
			return false;
		}
	}

	return true;
}

void WNS::startNetworking()
{
	std::string registrationServerEndpoint = "";

	wns::gui::Service* s = new Server(pyConfig.get<wns::pyconfig::View>("WNS.server"));
	server = dynamic_cast<wns::Server*>(s);
	server->setEventScheduler(wns::simulator::getEventScheduler());
	wns::simulator::getRegistry()->insert("WNS.GUI", s);
}

void WNS::stopNetworking()
{
	if (server)
	{
		server->deregisterRemote();
	}
}

void
WNS::abort()
{
	prematureAbort = true;
	wns::simulator::getEventScheduler()->queueCommand(SimControl::PrematureAbort(simctrl));
}

simTimeType
WNS::getMaxSimulationTime()
{
	return maxSimulationTime;
}

bool
WNS::isPrematureAbort()
{
	return prematureAbort;
}

void WNS::initWNS()
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

void WNS::run()
{
	this->simFunction();
}

void WNS::stopProbes()
{
	probeWriter.cancelPeriodicRealTimeout();

	MESSAGE_BEGIN(NORMAL, log, m, "Writing Probes");
	MESSAGE_END();

 	PDataBase* db = wns::simulator::getRegistry()->find<PDataBase*>("WNS.DataBase");
 	db->writeProbes();

        try {
                sr.writeStatus(true);
        }
        catch(...) {
                std::cerr << "couldn't write status file.\n";
        }
	sr.stop();

	// Trigger output for ProbeBusses
	if(probeBusRegistry != NULL)
	{
		probeBusRegistry->forwardOutput();
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

void WNS::simFunction()
{
	MESSAGE_BEGIN(NORMAL, log, m, "Start StatusReport");
	MESSAGE_END();
	this->sr.start(this->pyConfig.getView("WNS"));
	this->sr.writeStatus(false);

	// StartUp:
	std::list<module::Base*>::iterator itr;
	std::list<module::Base*>::iterator itrEnd=this->configuredModules.end();

	MESSAGE_BEGIN(NORMAL, log, m, "Start up modules");
	MESSAGE_END();
	for(itr=this->configuredModules.begin(); itr!=itrEnd; ++itr) {
		(*itr)->configure();
	}

	// Construct Nodes
	for(int ii=0; ii < this->pyConfig.len("WNS.nodes"); ++ii)
	{
		wns::pyconfig::View nodeView =
			this->pyConfig.get<wns::pyconfig::View>("WNS.nodes",ii);

		new wns::node::Node(&this->registry, nodeView);
	}

	// World created -> call onWorldCreated for each node
	for(wns::node::Registry::const_iterator itr = this->registry.begin();
	    itr != this->registry.end();
	    ++itr) {
		itr->second->onWorldCreated();
	}

	for(itr=this->configuredModules.begin(); itr!=itrEnd; ++itr) {
		(*itr)->startUp();
	}
	MESSAGE_BEGIN(NORMAL, log, m, "Start Persistency and Probes");
	MESSAGE_END();
	{
		double period =
			this->pyConfig.get<double>("WNS.probesWriteInterval");

		if(period != 0.0) {
			MESSAGE_BEGIN(NORMAL, log, m, "Start ProbeWriter");
			MESSAGE_END();

			this->probeWriter.startPeriodicTimeout(period, period);
		}
	}
	this->sr.writeStatus(false, "WNSStatusBeforeEventSchedulerStart.dat");
	MESSAGE_BEGIN(NORMAL, log, m, "Start Scheduler");
	MESSAGE_END();

	if(this->networkingEnabled())
	{
		server->doStartThread();
		server->log("<registerLogger><module>__GUICONTROL__</module><location>__STARTUPCOMPLETE__</location></registerLogger>");
	}

	if (!stepping)
	{
		wns::simulator::getEventScheduler()->start();
	}
	else
	{
		eventloop();
	}

	this->shutdown();

	MESSAGE_BEGIN(NORMAL, log, m, "Simulation finished");
	MESSAGE_END();
	MESSAGE_BEGIN(NORMAL, log, m, "End MSC Traces");
	MESSAGE_END();
	if(this->networkingEnabled())
	{
		MESSAGE_BEGIN(NORMAL, log, m, "Stopping networking");
		MESSAGE_END();
		this->stopNetworking();
	}
}

void WNS::eventloop()
{
	bool eventsPending = true;
	while (eventsPending)
	{
		pthread_mutex_lock(&mutex);
		if (stepCounter > 0)
		{
			stepCounter--;
			pthread_mutex_unlock(&mutex);
			eventsPending = wns::simulator::getEventScheduler()->processOneEvent();
		}
		else
		{
			pthread_mutex_unlock(&mutex);
			// Wait for 100ms
			usleep(100);
		}
	}
}

void WNS::increaseStepsBy(int n)
{
	std::cout << "Adding " << n << " to steps" << std::endl;
	pthread_mutex_lock(&mutex);
	this->stepCounter += n;
	pthread_mutex_unlock(&mutex);
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
