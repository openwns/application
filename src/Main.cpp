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
 * @author Marc Schinnenburg <marc@schinnenburg.com>
 */
int main(int argc, char* argv[])
{
    try
    {
        wns::WNS& wns = wns::WNSSingleton::Instance();
        wns.readCommandLine(argc, argv);
        wns.init();
        try
        {
            wns.run();
        }
        catch (...)
        {
            // if enabled, print the backtrace
            wns::simulator::getMasterLogger()->outputBacktrace();
            // throw on to catch outside
            throw;
        }
        wns.shutdown();
        return wns.status();
    }
    catch (const wns::Exception& exception)
    {
        std::stringstream message;
        message << exception.getBacktrace()
                << "openWNS: Caught " << wns::TypeInfo::create(exception) << ":\n\n"
                << exception.what();
        std::cerr << message.str() << "\n\n";
        exit(1);
    }
    catch (const std::exception& exception)
    {
        std::stringstream message;
        message << "openWNS: Caught " << wns::TypeInfo::create(exception) << ":\n\n"
                << exception.what();
        std::cerr << message.str() << "\n\n";
        exit(1);
    }
    catch (...)
    {
        std::cerr << "openWNS: An unknown exception occurred.\n";
        exit(1);
    }

    // if we reach this point, something went wrong
    return 1;
}

