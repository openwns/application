import CNBuildSupport
import wnsbase

commonEnv = CNBuildSupport.CNBSEnvironment(PROJNAME       = 'openWNS',
                                           AUTODEPS       = ['wns'],
                                           SHORTCUTS      = True,
                                           DEFAULTVERSION = True,
                                           BINARY         = True,
                                           FLATINCLUDES   = False,
                                           LIBS           = ['wns-1.0'],
                                           REVISIONCONTROL = wnsbase.RCS.Bazaar('..', 'openWNS-core', 'main', '1.0'),
                                           )

Return('commonEnv')
