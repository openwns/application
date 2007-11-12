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

#ifndef WNS_SIGNALHANDLERS_HPP
#define WNS_SIGNALHANDLERS_HPP

#include <string>

namespace wns { namespace signalhandler {
    /**
     * @brief This signal handler (if enabled) will catch segmentation faults
     * and automatically attach the GNU Debugger 'gdb'. To enable this feature
     * start WNS with the switch "-s".
     */
    class SegmentationViolation
    {
    public:
        SegmentationViolation(
            bool attachDebugger,
            const std::string& debuggerName,
            const std::string& programName);

        void
        operator()();

    private:
        bool attachDebugger_;
        std::string debuggerName_;
        std::string programName_;
    };

    /**
     * @brief catch abort signals
     */
    class Abort
    {
    public:
        void
        operator()();
    };

    /**
     * @brief catch interrupt signal
     */
    class Interrupt
    {
    public:
        void
        operator()();
    };

    /**
     *  @brief catch user defined signal 1
     */
    class UserDefined1
    {
    public:
        void
        operator()();
    };

} // namespace signalhandler
} // namespace wns

#endif // NOT defined WNS_SIGNALHANDLERS_HPP

