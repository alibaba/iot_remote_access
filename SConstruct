import os

env=Environment(ENV=os.environ)
for k,v in os.environ.items():
    env[k]=v
for k,v in ARGUMENTS.items():
    env[k]=v

if 'board' not in env:
    env['board'] = 'centos'

env.AppendUnique(CFLAGS=['-Wall' ,'-Werror'])
env.AppendUnique(LIBS=['pthread'])
env.AppendUnique(LIBPATH=['board/'+env['board']+'/lib/'])
env.AppendUnique(RPATH = os.getcwd() + '/board/'+env['board']+'/lib/')
env.AppendUnique(CPPPATH = ['src/utility/hash_table/', 
                    'src/utility/json/', 
                    'src/utility/misc/', 
                    'src/utility/log/', 
                    'src/utility/sha256/', 
                    'src/connectivity/', 
                    'board/'+env['board']+'/include/',
                    'board/'+env['board']+'/include/nopoll/'])

common = env.Object([Glob('src/core/*.c'), 
             Glob('src/utility/hash_table/*.c'), 
             Glob('src/utility/json/*.c'), 
             Glob('src/utility/log/*.c'), 
             Glob('src/utility/misc/*.c'), 
             Glob('src/utility/sha256/*.c'), 
             Glob('src/connectivity/*.c')])

prog_static = env.Program('RemoteTerminalDaemon_static',
            [common, 'board/'+env['board']+'/lib/libnopoll.a',
             'board/'+env['board']+'/lib/libssl.a',
             'board/'+env['board']+'/lib/libcrypto.a'],
        LIBS = env['LIBS'] + ['dl'],
        CFLAGS = env['CFLAGS'] + ['-Wl,--no-as-needed -ldl'])

strip_cmd = 'strip'
if 'STRIP' in env and len(env['STRIP']):
    strip_cmd='$STRIP'

env.AddPostAction(prog_static, strip_cmd + " $TARGET")

#run example
#CC=/home/yuehu/toolchain/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin/arm-linux-gnueabi-gcc STRIP=/home/yuehu/toolchain/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin/arm-linux-gnueabi-strip scons board=arm-eabi

