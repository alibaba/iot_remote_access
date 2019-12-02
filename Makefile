.PHONY: clean

INSTALL_PATH := ./build

BINARY := RemoteTerminalDaemon
BIN_PATH := ${INSTALL_PATH}/bin
CONFIG_FILE := ./src/config/remote_terminal.json
START_SHELL :=./src/start_for_dynamic.sh

board ?= centos

override CFLAGS += $(addprefix -I, $(shell find ./src/ -type d))
override CFLAGS += -Iboard/$(board)/include/nopoll
LDFLAGS := -Lboard/$(board)/lib/
LIBS := -lnopoll -lssl -lcrypto -pthread -ldl

SOURCES := $(shell find ./src -name '*.c')
OBJECTS := $(SOURCES:%.c=%.o)


all: target install 

install:
	@cd ${INSTALL_PATH}/ 1>/dev/null 2>/dev/null  || mkdir ${INSTALL_PATH}
	@cd ${BIN_PATH}/ 1>/dev/null 2>/dev/null ||  cp -r board/${board}/* ${INSTALL_PATH}
	@rm -rf ${BIN_PATH}/*
	@mv ${BINARY}* ${BIN_PATH} 1>/dev/null 2>/dev/null
	@cp ${CONFIG_FILE} ${BIN_PATH}
	@cp ${START_SHELL} ${BIN_PATH} &&  chmod a+x ${BIN_PATH}/*.sh

	@echo -e "\033[32m ########### Compile completed ###########\033[0m"
	@echo  ""

target: $(OBJECTS)
	$(CC) -static $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o ${BINARY}_static $(LIBS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(BINARY)_dynamic $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(HEADERDIR) -I$(dir $<) -c $< -o $@

clean:
	rm -rf $(OBJECTS) ${INSTALL_PATH} 1>/dev/null 2>/dev/null

#run example:
#make board=armv7 CC=/home/yuehu/toolchain/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin/arm-linux-gnueabi-gcc STRIP=/home/yuehu/toolchain/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin/arm-linux-gnueabi-strip

