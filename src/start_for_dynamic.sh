#!/bin/bash

WORK_DIR=$(cd $(dirname $0); pwd)
LIB_DIR=${WORK_DIR}/../lib
LD_LIBRARY_PATH=${LIB_DIR}:${LD_LIBRARY_PATH}

pk=$1
dn=$2
ds=$3

 ./RemoteTerminalDaemon_dynamic ${pk} ${dn} ${ds}
