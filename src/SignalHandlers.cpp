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
#include <execinfo.h>
#include <cxxabi.h>

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
    printBacktrace();

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
    printBacktrace();
    // that's the end, my friend ...
    return;
}


void
Interrupt::operator()()
{
    std::cout << "openWNS: caught signal 'SIGINT' (interrupt)\n";
    printBacktrace();
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

void
wns::signalhandler::printBacktrace()
{
    void* array[1000];
    size_t size;
    char** strings;

    size = backtrace (array, 1000);
    strings = backtrace_symbols (array, size);

    std::cout << "Backtrace " << size << " frames:\n";

    for (size_t ii = 0; ii < size; ++ii)
    {
        // ./openwns(_ZN3wns3WNS13SignalHandler11catchSignalEi+0xfc) [0x8068870]
        std::string tmp(strings[ii]);
        size_t openbrace = tmp.find("(");


        if (openbrace != std::string::npos)
        {
            size_t plus = tmp.find("+");
            size_t closebrace = tmp.find(")");
            std::string symbol = tmp.substr(openbrace+1, plus-openbrace-1);

            const size_t max = 10240;
            char buf[max + 1];
            size_t length = max;
            int result;
            abi::__cxa_demangle(symbol.c_str(), buf, &length, &result);
            buf[length] = '\0';
            std::string place = buf;
            tmp.erase(openbrace, closebrace-openbrace+1);
            std::cout << place << "        " << tmp << "\n";
        }
        else
        {
            std::cout << tmp << "\n";
        }
    }

    free (strings);
}
