#!/bin/bash

SystemType=`uname`
DefaultPackageVersion="20181202"

if [ "$#" != 3 ]; then
    echo "usage: ./RemoteTerminalDaemon ProductKey DeviceName DeviceSecret"
    exit 0
fi

echo "ProductKey: $1, DeviceName: $2, DeviceSecret: $3"

if [ "$SystemType" = "Darwin" ]; then
    sudo systemsetup -setremotelogin on &> /dev/null
    DefaultUrlPrefix="http://remote-access-oxs.oss-cn-shanghai.aliyuncs.com/%E8%AE%BE%E5%A4%87%E4%BE%A7%E5%8F%AF%E6%89%A7%E8%A1%8C%E7%A8%8B%E5%BA%8F/MacOS%E7%89%88/"
    BinaryPackage="RemoteAccessTerminal_Macos"
elif [ "$SystemType" = "Linux" ]; then
    sudo /usr/sbin/sshd
    DefaultUrlPrefix="http://remote-access-oxs.oss-cn-shanghai.aliyuncs.com/%E8%AE%BE%E5%A4%87%E4%BE%A7%E5%8F%AF%E6%89%A7%E8%A1%8C%E7%A8%8B%E5%BA%8F/Ubuntu%E7%89%88/"
    BinaryPackage="RemoteAccessTerminal_Ubuntu_64"
else
    echo "Your system is not support yet."
    exit 0
fi

Url=$DefaultUrlPrefix$BinaryPackage"_"$DefaultPackageVersion".tar.gz"
PackageName=$BinaryPackage"_"$DefaultPackageVersion".tar.gz"
ProgramName="RemoteTerminalDaemon_static"

if [ ! -d $BinaryPackage ]; then
    echo "downloading $Url..."
    curl -O $Url
    tar zxvf $PackageName
fi

cd ./$BinaryPackage

chmod +x ./$ProgramName &&./$ProgramName "$1" "$2" "$3"


