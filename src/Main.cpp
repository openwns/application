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
#include <WNS-CORE/DetailedProgressListener.hpp>
#include <WNS-CORE/curses/CursesTestRunner.h>

#include <WNS/gui/Service.hpp>
#include <WNS/logger/Master.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/CppUnit.hpp>
#include <WNS/speetcl/stateval.hpp>
#include <WNS/ldk/CommandProxy.hpp>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>

#undef ref
#undef deref

namespace wns {
	void
	breakpoint()
	{}
}

void
unexpectedHandler()
{
	if(wns::WNS::wns) {
		wns::WNS::wns->outputBacktrace();
	}
	std::cerr << "WNS: unexpected exception:\n";
	exit(1);
}

/**
 * @brief Creates an instance of WNS, reads command line parameters and starts
 * the simulation.
 *
 * In order to execute a simulation the following steps are necessary:
 * @li Create an instance of WNS
 * @li Read command line parameters (switches)
 * @li Read list of modules to be started (from config file using switch '-i' or
 * from command line)
 * @li Initialize SPEETCL (WNS::initSPEETCL())
 * @li Load Modules (WNS::loadModules())
 * @li Initialize modules, startup modules, create networking thread and let the
 * simulation run (WNS::run())
 * @li Wait until maximum simulation is reached
 * @li Shutdown Modules smoothley
 * @li Stop event scheduler
 * @li Shutdown WNS
 * @note To see how modules (written by you) are loaded have a look at
 * WNS::loadModules()
 * @note To see how modules (written by you) are initialized and started have a
 * look at WNS::simFunction()
 */
int main(int argc, char* argv[])
{
	wns::WNS::prog_name = std::string(argv[0]);
	wns::WNS *wns = new wns::WNS();

	wns->readCommandLineParameter(argc, argv);

	// startup simulation environment and load Modules
	wns->initWNS();
	wns->initSPEETCL();
	bool moduleLoadingSucceeded = wns->loadModules();

	wns::simulator::getResetSignal()->connect(&wns::ldk::CommandProxy::clearRegistries);

	// use "break wns::breakpoint" to stop after all Modules have been
	// loaded
	wns::breakpoint();

	// Unit tests are processed here
	if(wns->testingEnabled()) {
		std::vector<std::string> testNames = wns->getTestNames();

		CppUnit::TestFactoryRegistry& defaultRegistry = CppUnit::TestFactoryRegistry::getRegistry(wns::testsuite::Default());
		CppUnit::TestSuite* defaultSuite = new CppUnit::TestSuite("DefaultTests");
		defaultSuite->addTest(defaultRegistry.makeTest());
		defaultSuite->addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());

		// run the specified tests
		// Choose the testrunner:
		bool wasSuccessful = false;
		if (wns->getTestingStyle() == "text")
		{
			CppUnit::TextTestRunner runner;
			// Built tests (either all, or only specific ones given on the
			// command line)
			if(testNames.empty()) {
				runner.addTest(defaultSuite);
			} else {
				CppUnit::TestSuite* masterSuite = new CppUnit::TestSuite("AllTests");

				// register default tests
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

				for(std::vector<std::string>::iterator ii = testNames.begin();
				    ii != testNames.end();
				    ++ii) {
					runner.addTest(masterSuite->findTest(*ii));
				}
			}
			if(wns->isVerbose()) {
				DetailedProgressListener progress;
				runner.eventManager().addListener(&progress);
				wasSuccessful = runner.run("", false, true, false);
			} else {
				wasSuccessful = runner.run("", false);
			}
		}
		else if(wns->getTestingStyle() == "curses")
		{
			CppUnit::CursesTestRunner runner;
			runner.addTest(defaultRegistry.makeTest());
			wasSuccessful = runner.run();
		}
		else
		{
			wns::Exception e("No such style for testing UI: ");
			e << wns->getTestingStyle() << "\n"
			  << "Available are:\n"
			  << "  - text\n"
			  << "  - curses";
			throw e;
		}

		wns->stopProbes();
		wns->shutdown();
		delete wns;

		if(wasSuccessful && moduleLoadingSucceeded)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}

	if(!moduleLoadingSucceeded) {
		return 1;
	}

	wns->writeFingerprint();

	// in order for the backtrace to work, we need to override the
	// set_unexpected() handler previously set by SPEETCL
	std::set_unexpected(unexpectedHandler);

	// preserve abort status
	bool prematureAbort = false;
	// Finally run WNS
	// Run in try-block to be able to catch all exceptions
	try {
		wns->run();
		prematureAbort = wns->isPrematureAbort();
		delete wns;
	}
	// we catch everything in order to finally print the backtrace (if defined)
	catch (Error& e)
	{
		wns->outputBacktrace();
		std::cerr << "WNS: Caught SPEETCL library exception:\n\n"
			  << e.what() << std::endl << std::endl;
		exit(1);
	}
	catch (const std::exception& e)
	{
		wns->outputBacktrace();
		std::stringstream s;
		s << "WNS: Caught standard library exception:\n\n"
		  << e.what();
		std::cerr << s.str() << std::endl << std::endl;
		exit(1);
	}
	catch (...)
	{
		wns->outputBacktrace();
		std::cerr << "WNS: An uncaught and unknown exception occurred.\n";
		exit(1);
	}

	if (prematureAbort == true)
		return 2;
	else
		return 0;
}

/**
 * @mainpage W-NS - Wireless Network Simulator
 *
 * @section Preface Preface
 * This documentation provides comprehensive information on the source code of
 * W-NS as well as the overall concepts. Information on the source code may
 * be found on the left of this page under the item "Class Hierachy". Apart from
 * this some more documentation is available under the "Table of Contents".
 *
 * @section Introduction Introduction
 * Wireless Network Simulator (W-NS) is a platform that supports
 * stochastic, event driven, dynamic simulation of mobile radio networks for
 * performance evaluation at system level. However, W-NS itself is not a
 * simulator but needs to load modules in order to carry out a simulation.
 * A lot of different modules are available right now (RISE, UMTS, HiperLAN/2,
 * 802.11a/e/g) and even more will follow.
 *
 * The main idea of this platform is to support a modular design which makes it
 * easy to create, integrate and utilize modules for the performance evaluation
 * of mobile radio networks. The collaboration of the modules and exploitation
 * of the provided integration technologies enables communication engineers to
 * leverage module reuse and concentrate on their core competencies to create
 * new communication systems.
 *
 * W-NS is entirely written in C++.
 *
 * @section TOC Table of contents
 *
 * Besides the documentation of the classes some further documentation is
 * available:
 * - @link Requirements Requirements @endlink
 * - @link Obtaining Obtaining W-NS @endlink
 * - @link CompilingW-NS Compiling W-NS @endlink
 * - @link GeneratingDocu Generating this documentation @endlink
 * - @link HowItWorks How it works - Interaction of W-NS with other modules @endlink
 * - @link ExemplaryResults Exemplary Results @endlink
 * - @link todo ToDo @endlink
 *
 * To understand how W-NS works start at @link main() int main(int argc, char* argv[])@endlink!
 */

/**
 * @page Requirements Requirements
 * @section HardwareRequirements Hardware Requirements
 * @subsection Architecture
 * @li Everything that is supported by gcc-3.3 (or greater)
 *
 * So far the platform has been tested on UltraSPARC II and x86 machines.
 *
 * @section SoftwareRequirements Software Requirements
 * @note All the software used for this project is written under the GPL and
 * therefor freely available.
 * @subsection Mandatory
 * @subsubsection Compilation
 * @li <a href="http://gcc.gnu.org">gcc-3.3</a> (or greater)
 * @li <a href="http://www.scons.org">SCons</a> (SConstruct, a 'make' replacement)
 * @li <a href="http://www.python.org">Python 2.3</a> (or greater)
 *
 * @subsubsection AdditionLibraries Addition Libraries
 * @li <a href="http://www.comnets.rwth-aachen.de">SPEETCL 5.23</a> (Library for event driven simulation)
 * @li <a href="http://xml.apache.org">XERCES 2.6</a> (Library for XML decoding)
 *
 * @subsection Optional
 * @subsubsection Fetch the source from repository:
 * @li <a href="http://www.gnuarch.org">tla 1.2</a> (or greater)
 * @subsubsection GeneratingDocumentation Generating Documentation
 * @li <a href="http://www.doxygen.org">doxygen</a>
 */

/**
 * @page Obtaining Obtaining W-NS
 * @section ObtainingStandalone Obtaining stand alone version
 *
 * The current version of W-NS is available from the ComNets TLA archive:
 * - archive:  software@comnets.rwth-aachen.de--2005
 * - location: http://user:pass@arch.comnets.rwth-aachen.de/software-2005
 * - category: wns-core
 * - branch:   main
 * - revision: 1.0
 *
 * To retrieve a copy from the archive you would type:
 * @verbatim tla get -A software@comnets.rwth-aachen.de--2005 wns-core--main--1.0 wns-core @endverbatim
 *
 * After executing the above line you will have a copy of the current version in
 * the directory:
 * @verbatim ./wns-core @endverbatim
 */


/**
 * @page HowItWorks How it works
 * @image html concept_small.png "W-NS with different Modules"
 */

/**
 * @page ExemplaryResults Exemplary Results
 * @image html CIRoverArea.png "CIR over area"
 * @image html CIRvsDistance.png "CIR vs. distance"
 */

/**
 * @page CompilingW-NS Compiling W-NS
 * @section CompilingStandalone Compiling stand alone version
 *
 * W-NS is compiled using SConstuct (SCons) and CNBuildSupport (for
 * SCons). All you have to do is to execute:
 * @verbatim scons build-bin-dbg @endverbatim
 *
 * After the compiliation finished you should find a binary under:
 * @verbatim ./bin/dbg/wns-core-0.3 @endverbatim
 *
 * @note You have to make sure that the include path to SPEETCL and INTERFACE
 * are set correct in your <code>private.py</code>. Otherwise compilation will
 * fail.
 */

/**
 * @page GeneratingDocu Generating this documentation
 *
 * This documentation is available as HTML and PDF version. In order to generate
 * these documentations execute:
 * @verbatim scons docu @endverbatim
 * This will result in to directories with the respective documentation:
 * @verbatim ./doxydoc/latex
 ./doxydoc/html @endverbatim
 * To use the HTML documentation point your browser to:
 * @verbatim ./doxydoc/html/index.html @endverbatim
 * To use the PDF version you have to execute the following:
 * @verbatim cd ./doxydoc/latex
 make all
 acroread refman.pdf @endverbatim
*/


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

