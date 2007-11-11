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

#include "WNS.hpp"
#include "SimControl.hpp"

#include <WNS/pyconfig/View.hpp>
#include <WNS/Assure.hpp>

#include <WNS/speetcl/event.hpp>

using namespace wns;

SimControl::SimControl(WNS* _wns) :
	PeriodicTimeout(),
	wns(_wns),
	log("WNS", "SimControl", wns::simulator::getMasterLogger()),
	fastShutdown(false)
{
	assure(this->wns, "Invalid WNS (NULL)");
	this->startPeriodicTimeout(1, 1);
}

SimControl::~SimControl()
{
	if (this->hasPeriodicTimeoutSet())
		this->cancelPeriodicTimeout();
}

void
SimControl::periodically()
{
	MESSAGE_SINGLE(NORMAL, log, "--- MARK ---");
}

void
SimControl::setMaxSimTime(simTimeType endtime, bool _fastShutdown)
{
	wns::simulator::getEventScheduler()->sendDelay(new Shutdown(this), endtime);
	this->fastShutdown = _fastShutdown;
}

SimControl::Shutdown::Shutdown(SimControl* _simControl) :
	Event(),
	simControl(_simControl)
{
	assure(this->simControl, "Invalid SimControl (NULL)");
}

void
SimControl::Shutdown::execute()
{
	// stopping Probes
	this->simControl->wns->stopProbes();

	// fast shutdown (no Nodes and Modules will be deleted)
	if(this->simControl->fastShutdown) {
		MESSAGE_SINGLE(NORMAL, this->simControl->log, "fastShutdown ... bye");
		exit(0);
	}

	// stopping EventScheduler
	MESSAGE_SINGLE(NORMAL, this->simControl->log, "Stopping scheduler - final events are now being processed!");
	wns::simulator::getEventScheduler()->stop();
}

SimControl::PrematureAbort::PrematureAbort(SimControl* _simControl) :
	simControl(_simControl)
{
	assure(this->simControl, "Invalid SimControl (NULL)");
}

void
SimControl::PrematureAbort::operator()()
{
	wns::simulator::getEventScheduler()->sendNow(new SimControl::Shutdown(this->simControl));
}

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

