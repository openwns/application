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

namespace wns {
    /**
     * @brief function that returns false only the first time it is being
     * called.
     */
    bool
    abortCalled();

    /**
     * @brief This signal handler (if enabled) will catch segmentation faults
     * and automatically attach the GNU Debugger 'gdb'. To enable this feature
     * start WNS with the switch "-s".
     */
    void
    catch_segv(int);

    /**
     * @brief catch abort signals
     */
    void
    catch_abrt(int);

    /**
     * @brief catch interrupt signal
     */
    void
    catch_int(int);

    /**
     *  @brief catch interrupt signal
     */
    void
    catch_usr1(int signum);


    /**
     * @brief catch signal USR2, sent by SGE shortly before SIGKILL when
     * exceeding resource limits. It leads to a premature end of the (otherwise
     * smoothly running) simulation.
     */
    void
    catch_usr2(int signum);

    /**
     * @brief catch signal SIGXCPU
     */
    void
    catch_xcpu(int signum);

} // namespace wns

#endif // NOT defined WNS_SIGNALHANDLERS_HPP

