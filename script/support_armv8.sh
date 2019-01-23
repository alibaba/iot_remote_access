#!/bin/bash

########## 从这行开始，均为需要用户根据交叉工具链的实际情况修改 ############

_ToolChainRootDirectory="/tmp/gcc-linaro-7.3.1-2018.05-i686_aarch64-linux-gnu"

_CrossPrefix="aarch64-linux-gnu-"

# _CC 为交叉编译所需要的gcc的绝对路径.
_CC="${_CrossPrefix}gcc"

_AR="${_CrossPrefix}ar"

_RANLIB="${_CrossPrefix}ranlib"

_Host="i686-pc-linux-gnu"

_Target="aarch64-linux-gnu"

_TargetBit="64"

_NewBoardName="armv8"

########################### 用户自行修改结束  ##############################

_CurDir=`realpath "$0" | sed 's|\(.*\)/.*|\1|'`
_RootDir=${_CurDir}/../

echo "Generate board directory..."
mkdir -p "${_RootDir}/board/${_NewBoardName}/"

export PATH="${_ToolChainRootDirectory}/bin:$PATH"

#echo "Compile openssl..."
cd "${_RootDir}/dependencies/"

rm -rf openssl-1.0.2l/
tar zxf openssl-1.0.2l.tar.gz
cd openssl-1.0.2l/
./Configure "linux-generic${_TargetBit}" --prefix="${_RootDir}/board/${_NewBoardName}/" shared
make CC="${_CC}" AR="${_AR} r" RANLIB="${_RANLIB}" -j8 build_libs build_apps
make install_sw
cd -

echo "Compile nopoll..."

rm -rf nopoll
tar zxf nopoll-0.4.6.tar.gz
cd nopoll
./autogen.sh --prefix="${_RootDir}/board/${_NewBoardName}/" CC="${_CC}" --host=${_Target} CFLAGS="-I${_RootDir}/board/${_NewBoardName}/include" --disable-shared LDFLAGS="-L${_RootDir}/board/${_NewBoardName}/lib/" LIBS="-lcrypto"
make CFLAGS="-I${_RootDir}/board/${_NewBoardName}/include" LDFLAGS="-L${_RootDir}/board/${_NewBoardName}/lib/ -ldl -lpthread"
make install
cd -
