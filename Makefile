CC ?= cc
CFLAGS ?= -Wall -pedantic -O2
LDFLAGS ?= -fPIC

SRC = envs.c hooks.c
TARGET = modEnv.so

BUILD_ID = $(shell date +%s)
DANTI_REHOOK ?= 1
DEBUG ?= 0
APPLY_TO ?= ""

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -DBUILD_ID=\"$(BUILD_ID)\" -DANTI_REHOOK=$(DANTI_REHOOK) -DDEBUG=$(DEBUG) -DAPPLY_TO=$(APPLY_TO) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^

clean:
	rm -f $(TARGET)

.PHONY: all clean
