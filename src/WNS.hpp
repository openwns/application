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

#ifndef WNS_WNS_HPP
#define WNS_WNS_HPP

#include <WNS/pyconfig/Parser.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/Singleton.hpp>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <vector>
#include <string>

namespace wns {

    /**
     * @brief Run-time environment of WNS
     * @author Marc Schinnenburg <marc@schinnenburg.net>
     *
     * The following mehtods are called in this order:
     *  - WNS()
     *  - readCommandLine()
     *  - init()
     *  - run()
     *  - shutdown()
     *  - status()
     *  - ~WNS()
     *
     * The behaviour of the different methods will be configurable in the future
     */
    class WNS
    {

        friend class wns::DefaultCreation<WNS>;

        // needs to be vector to work with boost
        typedef std::vector<std::string> TestNameContainer;
        typedef std::vector<std::string> PyConfigPatchContainer;

        /**
         * @brief Internal signal handler
         * @author Marc Schinnenburg <marc@schinnenburg.net>
         */
        class SignalHandler
        {
            typedef boost::signal0<void> Handler;
            typedef wns::container::Registry<int, Handler*, wns::container::registry::DeleteOnErase> Map;

        public:
            /**
             * @brief Default c'tor
             */
            SignalHandler();

            /**
             * @brief Remove all registered handlers and restore default handler
             */
            virtual
            ~SignalHandler();

            /**
             * @brief Add a signal handler
             *
             * If a signal handler for signum is already registerd it will be
             * overwritten
             */
            template <typename HANDLER>
            void
            addSignalHandler(int signum, HANDLER handler)
            {
                if(!map_.knows(signum))
                {
                    map_.insert(signum, new Handler());
                }
                map_.find(signum)->disconnect_all_slots();
                map_.find(signum)->connect(handler);
                signal(signum, SignalHandler::catchSignal);
            }

            /**
             * @brief Remove signal according signal number, installs default
             * signal handler again.
             */
            void
            removeSignalHandler(int signum);

            /**
             * @brief Remove all registered handlers and restore default handler
             */
            void
            removeAllSignalHandlers();

        private:
            /**
             * @brief Copy c'tor is forbidden
             */
            SignalHandler(const SignalHandler&);

            /**
             * @brief Assignment is forbidden
             */
            void
            operator =(const SignalHandler&);

            /**
             * @brief Dispatch signals
             */
            static void
            catchSignal(int signum);

            /**
             * @brief Map the signals to the signal handlers
             */
            Map map_;
        };

    public:
        /**
         * @brief Reads the command line parameters
         */
        virtual void
        readCommandLine(int argc, char* argv[]);

        /**
         * @brief Configure stuff that belongs to WNS
         */
        virtual void
        init();

        /**
         * @brief Run the simulation
         */
        virtual void
        run();

        /**
         * @brief Shutdown everything
         */
        virtual void
        shutdown();

        /**
         * @brief Return status code
         */
        virtual int
        status() const;

    private:
        /**
         * @brief Handle unexpected exceptions
         */
        static void
        unexpectedHandler();

        // C'tor and d'tor are private and may only be accessed by the SingletonHolder
        /**
         * @brief Default Constructor
         */
        WNS();

        /**
         * @brief Default Destructor
         */
        virtual
        ~WNS() throw();

        /**
         * @brief Copy c'tor is forbidden
         */
        WNS(const WNS&);

        /**
         * @brief Assignment is forbidden
         */
        void
        operator =(const WNS&);

        /**
         * @brief Returns the path to pyconfig
         */
        std::string
        getPythonPath() const;

        /**
         * @brief The status code of openWNS
         *
         * 0 - everything fine
         *
         * 1 - an error occured
         */
        int status_;

        /**
         * @brief Name of the configuration file
         */
        std::string configFile_;

        /**
         * @brief Configuration as parsed from configFile_
         */
        pyconfig::Parser configuration_;

        /**
         * @brief Be chatty
         */
        bool verbose_;

        /**
         * @brief Is wns in testing mode?
         */
        bool testing_;

        /**
         * @brief If special tests have been defined they are stored here
         */
        TestNameContainer testNames_;

        /**
         * @brief Holds all PyConfig patches
         */
        PyConfigPatchContainer pyConfigPatches_;

        /**
         * @brief Defines the valid options for the command line
         */
        boost::program_options::options_description options_;

        /**
         * brief Holds the command line parameters
         */
        boost::program_options::variables_map arguments_;

        /**
         * @brief Our own name (as from argv[0])
         */
        std::string programName_;

        /**
         * @brief In case we should launch a debugger, this is the name of it
         */
        std::string debuggerName_;

        /**
         * @brief Are we expected to launch a debugger on segfault?
         */
        bool attachDebugger_;

        /**
         * @brief Manages the signal handlers
         */
        SignalHandler signalHandler_;
    };

    /**
     * @brief WNS is a singleton
     */
    typedef wns::SingletonHolder<WNS> WNSSingleton;
}
#endif // NOT defined WNS_WNS_HPP

