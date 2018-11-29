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
env.AppendUnique(LIBPATH=['board/'+env['board']+'/prebuilt/nopoll/lib/', 'board/'+env['board']+'/prebuilt/openssl/lib/'])
env.AppendUnique(RPATH = os.getcwd() + '/board/'+env['board']+'/prebuilt/openssl/lib/')
env.AppendUnique(CPPPATH = ['src/utility/hash_table/', 
                    'src/utility/json/', 
                    'src/utility/misc/', 
                    'src/utility/log/', 
                    'src/utility/sha256/', 
                    'src/connectivity/', 
                    'board/'+env['board']+'/prebuilt/nopoll/include/', 'board/'+env['board']+'/prebuilt/openssl/include/'])

common = env.Object([Glob('src/core/*.c'), 
             Glob('src/utility/hash_table/*.c'), 
             Glob('src/utility/json/*.c'), 
             Glob('src/utility/log/*.c'), 
             Glob('src/utility/misc/*.c'), 
             Glob('src/utility/sha256/*.c'), 
             Glob('src/connectivity/*.c')])

prog_static = env.Program('RemoteTerminalDaemon_static',
            [common, 'board/'+env['board']+'/prebuilt/nopoll/lib/libnopoll.a',
             'board/'+env['board']+'/prebuilt/openssl/lib/libssl.a',
             'board/'+env['board']+'/prebuilt/openssl/lib/libcrypto.a'],
        LIBS = env['LIBS'] + ['dl'],
        CFLAGS = env['CFLAGS'] + ['-Wl,--no-as-needed -ldl'])


prog = env.Program('RemoteTerminalDaemon',
            common,
            LIBS= env['LIBS'] + ['nopoll', 'ssl', 'crypto'])

strip_cmd = 'strip'
if 'STRIP' in env and len(env['STRIP']):
    strip_cmd='$STRIP'

env.AddPostAction(prog, strip_cmd + " $TARGET")
env.AddPostAction(prog_static, strip_cmd + " $TARGET")
