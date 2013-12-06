import os
import platform
import sys
Import('env')
files = SConscript('config/libfiles.py')

coreEnv = env.Clone()
LIBS = ['wns'] + coreEnv['externalLIBS']

if coreEnv['static']:
    coreEnv.Append(LINKFLAGS = '-Wl,--whole-archive')
    for lib in coreEnv['libraries']:
        coreEnv.Append(LINKFLAGS = '-l'+lib)
    coreEnv.Append(LINKFLAGS = '-Wl,--no-whole-archive')

if sys.platform == 'darwin':
    coreEnv.Append(LINKFLAGS = '-Wl,-flat_namespace -Wl,-force_flat_namespace ')
    coreEnv.Append(LINKFLAGS = '-Wl,-rpath '+os.path.join(coreEnv.installDir,'lib'))
elif not platform.system().startswith("CYGWIN"):
    coreEnv.Append(LINKFLAGS = '-Wl,-disable-new-dtags')

prog = coreEnv.Program('openwns', files, LIBS = LIBS)
if coreEnv['static']:
    for lib in coreEnv['libraries']:
        Depends(prog, os.path.join(coreEnv.installDir,'lib', 'lib'+lib+'.a'))
else:
    coreEnv.Append(RPATH = os.path.join(env.installDir, 'lib'))
coreEnv.Install(os.path.join(env.installDir, 'bin'), prog )
