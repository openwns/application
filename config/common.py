import os
import CNBuildSupport
from CNBuildSupport import CNBSEnvironment
import wnsbase.RCS as RCS

commonEnv = CNBSEnvironment(PROJNAME       = 'openWNS',
                            AUTODEPS       = ['wns'],
                            SHORTCUTS      = True,
                            DEFAULTVERSION = True,
                            BINARY         = True,
                            FLATINCLUDES   = False,
                            LIBS           = ['wns-3.0',
                                              'cppunit',
                                              'pthread',
                                              'python2.4',
                                              'dl',
                                              'boost_program_options',
                                              'boost_signals',
                                              'gsl',
                                              'gslcblas',
                                              ],
                            REVISIONCONTROL = RCS.Bazaar('..', 'openWNS-core', 'main', '1.0'),
                            )

Return('commonEnv')
