CC ?= cc
CFLAGS ?= -Wall -pedantic -O2
LDFLAGS ?= -fPIC

SRC = envs.c hooks.c
TARGET = modEnv.so

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -DDEBUG=$(DEBUG) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^

clean:
	rm -f $(TARGET)

.PHONY: all clean
