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
#include <WNS/TypeInfo.hpp>
#include <WNS/simulator/ISimulator.hpp>

/**
 * @brief Creates an instance of WNS, reads command line parameters and starts
 * the simulation.
 */
int main(int argc, char* argv[])
{
	try
	{
		wns::WNS& wns = wns::WNSSingleton::Instance();
		wns.readCommandLine(argc, argv);
		wns.init();
		wns.run();
		wns.shutdown();
		return wns.status();
	}
	// we catch everything in order to finally print the backtrace (if
	// defined)
	// since wns::Exception is derived from std::exception we don't need to
	// make a difference here
	catch (const std::exception& exception)
	{
		wns::simulator::getMasterLogger()->outputBacktrace();
		std::stringstream ss;
		ss << "openWNS: Caught " << wns::TypeInfo::create(exception) << ":\n\n" << exception.what();
		std::cerr << ss.str() << "\n\n";
		exit(1);
	}
	catch (...)
	{
		wns::simulator::getMasterLogger()->outputBacktrace();
		std::cerr << "openWNS: An unknown exception occurred.\n";
		exit(1);
	}

	// if we reach this point, something went wrong
	return 1;
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

