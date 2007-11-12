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
 * @author Marc Schinnenburg <marc@schinnenburg.net>
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

