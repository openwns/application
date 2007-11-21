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
#include <WNS/Backtrace.hpp>

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
    std::cerr << "openWNS: caught signal 'SIGSEGV' (segmentation violation)\n";

    Backtrace backtrace;
    backtrace.snapshot();
    std::cerr << backtrace;

    if (attachDebugger_)
    {
        int pid = getpid();

        std::cerr << "Attaching " << debuggerName_ << " to process ID " << pid << "\n";

        std::stringstream debuggerCall;
        debuggerCall << debuggerName_ << " " << programName_ << " " << pid << std::ends;
        system(debuggerCall.str().c_str());
    }

    exit(1);
}


void
Abort::operator()()
{
    std::cerr << "openWNS: caught signal 'SIGABRT' (abort)\n";
    Backtrace backtrace;
    backtrace.snapshot();
    std::cerr << backtrace;
    // that's the end, my friend ...
    return;
}


void
Interrupt::operator()()
{
    std::cerr << "openWNS: caught signal 'SIGINT' (interrupt)\n";
    Backtrace backtrace;
    backtrace.snapshot();
    std::cerr << backtrace;
    exit(1);
}
