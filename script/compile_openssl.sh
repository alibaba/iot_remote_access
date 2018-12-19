#!/bin/bash

_ToolChain="/home/yuehu/toolchain/gcc-linaro-7.3.1-2018.05-i686_arm-eabi"

_RootDir=`realpath "$0" | sed 's|\(.*\)/.*|\1|'`
_CFLAGS="-I${_ToolChain}/include -I${_RootDir}"
_LDFLAGS="-L${_ToolChain}/lib"
_CC="${_ToolChain}/bin/arm-eabi-gcc"

./Configure linux-generic32 --prefix=/home/yuehu/github/iot_remote_access/board/linaro_arm_v7_32/prebuilt/openssl/
CC=${_CC} CFLAGS=${_CFLAGS} LDFLAGS=${_LDFLAGS} make
CC=${_CC} CFLAGS=${_CFLAGS} LDFLAGS=${_LDFLAGS} make install

