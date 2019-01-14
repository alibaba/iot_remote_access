.PHONY: clean

BINARY = RemoteTerminalDaemon 

board ?= centos

override CFLAGS += $(addprefix -I, $(shell find ./src/ -type d))
override CFLAGS += -Iboard/$(board)/include/nopoll
LDFLAGS := -Lboard/$(board)/lib/
LIBS := -lnopoll -lssl -lcrypto -pthread -ldl

SOURCES := $(shell find ./src -name '*.c')
OBJECTS := $(SOURCES:%.c=%.o)

$(BINARY): $(OBJECTS)
	echo $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(BINARY) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(HEADERDIR) -I$(dir $<) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(BINARY) 

#run example:
#make board=armv7 CC=/home/yuehu/toolchain/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin/arm-linux-gnueabi-gcc STRIP=/home/yuehu/toolchain/gcc-linaro-7.3.1-2018.05-i686_arm-linux-gnueabi/bin/arm-linux-gnueabi-strip

