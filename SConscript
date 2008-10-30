import os
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
    LIBS += coreEnv['libraries']
    coreEnv.Append(LINKFLAGS = '-Wl,--whole-archive')
LIBS = list(set(LIBS))
prog = coreEnv.Program('openwns', files, LIBS = LIBS)
coreEnv.Append(RPATH = os.path.join(env.installDir, 'lib'))
coreEnv.Install(os.path.join(env.installDir, 'bin'), prog )
