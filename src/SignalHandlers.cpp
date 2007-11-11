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

#include <WNS-CORE/SignalHandlers.hpp>
#include <WNS-CORE/WNS.hpp>

#include <WNS/module/Base.hpp>

#include <iostream>

using namespace wns;


bool
wns::abortCalled()
{
	static bool value = false;
	bool alreadyCalled = value;
	value = true;
	return alreadyCalled;
}


void
wns::catch_segv(int)
{
    std::cout << "WNS caught Segfault\n";

    if (WNS::wns != NULL)
    {
        WNS::wns->stopNetworking();
    }

    if (WNS::outputBT == true)
    {
        wns::simulator::getMasterLogger()->outputBacktrace();
    }

    if (WNS::attachGDB == true)
    {
        std::cout << "Attaching " << WNS::gdb_name << " to process ..." << std::endl;
        std::stringstream str;
        str << WNS::gdb_name << " "<< WNS::prog_name << " "
            << getpid() << std::ends;
        system(str.str().c_str());
    }

    exit(1);
}


void
wns::catch_abrt(int)
{
    if (WNS::wns != NULL)
    {
        WNS::wns->stopNetworking();
    }
    exit(1);
}


void
wns::catch_int(int)
{
    std::cout << "WNS caught SIG INT\n";

    if (WNS::wns != NULL)
    {
        WNS::wns->stopNetworking();
    }

    if (WNS::outputBT == true)
    {
        wns::simulator::getMasterLogger()->outputBacktrace();
    }
    exit(1);
}


void
wns::catch_usr1(int signum)
{
    assert(signum == SIGUSR1);

    if (WNS::outputBT == true)
    {
        std::cout << std::endl
                  << "==================================================================" << std::endl
                  << "WNS caught SIG USR1 (" << signum << ")," << std::endl
                  << "The current simTime is: " << wns::simulator::getEventScheduler()->getTime()
                  << " [s]."<< std::endl
                  << "Now writing logger Backtrace." << std::endl
                  << "==================================================================" << std::endl;

        wns::simulator::getMasterLogger()->outputBacktrace();

        std::cout << std::endl
                  << "==================================================================" << std::endl
                  << " End of Backtrace, continuing execution." << std::endl
                  << "==================================================================" << std::endl;

    }

    return;
}


void
wns::catch_usr2(int signum)
{
    assert(signum == SIGUSR2);
    if (abortCalled() == false)
    {
        std::cout << std::endl
                  << "==================================================================" << std::endl
                  << "WNS caught SIG USR2 (" << signum << ")," << std::endl
                  << "it was aborted by SGE because resource limits (memory or CPU-time)" << std::endl
                  << "were violated." << std::endl << std::endl
                  << "The current simTime is: " << wns::simulator::getEventScheduler()->getTime()
                  << " [s]."<< std::endl
                  << "The simulator is now shut down, Final probes are being written." << std::endl
                  << "==================================================================" << std::endl;


        // Properly end the simulation now
        if (WNS::wns != NULL)
            WNS::wns->abort();
    }
    else
        return;
}


void
wns::catch_xcpu(int signum)
{
    assert(signum == SIGXCPU);

    if (abortCalled() == false)
    {
        std::cout << std::endl
                  << "==================================================================" << std::endl
                  << "WNS caught SIG XCPU (" << signum << ")," << std::endl
                  << "it was aborted by SGE because resource limits (memory or CPU-time)" << std::endl
                  << "were violated." << std::endl << std::endl
                  << "The current simTime is: " << wns::simulator::getEventScheduler()->getTime()
                  << " [s]."<< std::endl
                  << "The simulator is now shut down, Final probes are being written." << std::endl
                  << "==================================================================" << std::endl;


        // Properly end the simulation now
        if (WNS::wns != NULL)
        {
            WNS::wns->abort();
        }
    }
    else
    {
        return;
    }
}

