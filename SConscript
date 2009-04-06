import os
import platform
Import('env')
files = SConscript('config/libfiles.py')

coreEnv = env.Clone()

LIBS = [ 'wns', 
         'cppunit',
         'python2.5',
         'boost_program_options',
         'boost_signals', 
         'boost_date_time']

if coreEnv['static']:
    coreEnv.Append(LINKFLAGS = '-Wl,--whole-archive')
    for lib in coreEnv['libraries']:
        coreEnv.Append(LINKFLAGS = '-l'+lib)
    coreEnv.Append(LINKFLAGS = '-Wl,--no-whole-archive')

if not platform.system().startswith("CYGWIN"):
    coreEnv.Append(LINKFLAGS = '-Wl,-disable-new-dtags')

LIBS = list(set(LIBS))
prog = coreEnv.Program('openwns', files, LIBS = LIBS)
if coreEnv['static']:
    for lib in coreEnv['libraries']:
        Depends(prog, os.path.join(coreEnv.installDir,'lib', 'lib'+lib+'.a'))
else:
    coreEnv.Append(RPATH = os.path.join(env.installDir, 'lib'))
coreEnv.Install(os.path.join(env.installDir, 'bin'), prog )
