.PHONY: clean

BINARY = RemoteTerminalDaemon 

BOARD_TYPE = centos

CFLAGS := $(addprefix -I, $(shell find ./src/ -type d))
CFLAGS += -Iboard/$(BOARD_TYPE)/prebuilt/nopoll/include/ -Iboard/$(BOARD_TYPE)/prebuilt/openssl/include/
LDFLAGS := -Lboard/$(BOARD_TYPE)/prebuilt/nopoll/lib/ -Lboard/$(BOARD_TYPE)/prebuilt/openssl/lib/
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

