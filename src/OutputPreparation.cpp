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

#include <WNS-CORE/OutputPreparation.hpp>
#include <WNS/DateTime.hpp>

#include <WNS/speetcl/generr.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <iomanip>

using namespace wns;

STATIC_FACTORY_REGISTER(Move, OutputPreparationStrategy, "Move");
STATIC_FACTORY_REGISTER(Delete, OutputPreparationStrategy, "Delete");

void
Move::prepare(const std::string& path)
{
	struct stat buf;

	if(-1 == lstat(path.c_str(), &buf)) {
			// we encountered problems accessing the output directory.
			// the only acceptable error here is ENOENT (the directory
			// does not exist.)
			// all other errors should probably terminate the simulation.

		if(errno != ENOENT) {
			throw(GenErr("Couldn't access output directory."));
		}
	} else {
			// now we know the output path already exists.
			// let's just move it out of our way.

		struct tm mtime;
		if(NULL == localtime_r(&buf.st_mtime, &mtime)) {
				// i can hardly think of any error causing this function call
				// to fail.
			throw(GenErr("Please report this exception to osz@illator.de."));
		}

		std::stringstream ss;
		ss << path
		   << "." << DateTime(buf.st_mtime).getString();

		rename(path.c_str(), ss.str().c_str());
	}

	system(("mkdir -p " + path).c_str());
} // Move::prepare


void
Delete::prepare(const std::string& path)
{
	system(("rm -fr " + path).c_str());
	system(("mkdir " + path).c_str());
} // Delete::prepare


/*
  Local Variables:
  mode: c++
  fill-column: 80
  c-basic-offset: 4
  c-tab-always-indent: t
  indent-tabs-mode: t
  tab-width: 4
  End:
*/

