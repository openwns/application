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

#include <OPENWNS/SignalHandlers.hpp>
#include <WNS/simulator/ISimulator.hpp>
#include <WNS/logger/Master.hpp>

#include <iostream>

using namespace wns::signalhandler;

SegmentationViolation::SegmentationViolation(
    bool attachDebugger,
    const std::string& debuggerName,
    const std::string& programName) :
    attachDebugger_(attachDebugger),
    debuggerName_(debuggerName),
    programName_(programName)
{
}

void
SegmentationViolation::operator()()
{
    std::cout << "openWNS: caught signal 'SIGSEGV' (segmentation violation)\n";

    // output backtrace if available
    wns::simulator::getMasterLogger()->outputBacktrace();

    if (attachDebugger_)
    {
        int pid = getpid();

        std::cout << "Attaching " << debuggerName_ << " to process ID " << pid << "\n";

        std::stringstream debuggerCall;
        debuggerCall << debuggerName_ << " " << programName_ << " " << pid << std::ends;
        system(debuggerCall.str().c_str());
    }

    exit(1);
}


void
Abort::operator()()
{
    std::cout << "openWNS: caught signal 'SIGABRT' (abort)\n";
    // that's the end, my friend ...
    return;
}


void
Interrupt::operator()()
{
    std::cout << "openWNS: caught signal 'SIGINT' (interrupt)\n";
    // output backtrace if available
    wns::simulator::getMasterLogger()->outputBacktrace();
    exit(1);
}


void
UserDefined1::operator()()
{
    std::cout << "openWNS: caught signal 'SIGUSR1' (user defined signal 1),\n";

    std::cout << "\n==================================================================\n"
              << "The current simTime is: " << wns::simulator::getEventScheduler()->getTime()
              << " [s].\n"
              << "Now writing logger Backtrace (if enabled)\n."
              << "==================================================================\n";

    wns::simulator::getMasterLogger()->outputBacktrace();

    std::cout << "\n==================================================================\n"
              << " End of Backtrace, continuing execution.\n"
              << "==================================================================\n";

    // simply continue
    return;
}
