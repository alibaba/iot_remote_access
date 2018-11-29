.PHONY: clean

BINARY = RemoteTerminalDaemon 

board = centos

override CFLAGS += $(addprefix -I, $(shell find ./src/ -type d))
override CFLAGS += -Iboard/$(board)/prebuilt/nopoll/include/ -Iboard/$(board)/prebuilt/openssl/include/
LDFLAGS := -Lboard/$(board)/prebuilt/nopoll/lib/ -Lboard/$(board)/prebuilt/openssl/lib/
LIBS := -lnopoll -lssl -lcrypto -pthread

SOURCES := $(shell find ./src -name '*.c')
OBJECTS := $(SOURCES:%.c=%.o)

$(BINARY): $(OBJECTS)
	echo $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(BINARY) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(HEADERDIR) -I$(dir $<) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(BINARY) 

