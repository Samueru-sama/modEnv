CC ?= cc
CFLAGS ?= -Wall -pedantic -O2
LDFLAGS ?= -fPIC

SRC = envs.c hooks.c
TARGET = modEnv.so

all: $(TARGET)

BUILD_ID = $(shell date +%s)
DANTI_REHOOK ?= 1
DEBUG ?= 0

$(TARGET): $(SRC)
	$(CC) -DBUILD_ID=\"$(BUILD_ID)\" -DANTI_REHOOK=$(DANTI_REHOOK) -DDEBUG=$(DEBUG) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^

clean:
	rm -f $(TARGET)

.PHONY: all clean
