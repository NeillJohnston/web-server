export BIN = $(shell pwd)/bin
SERVER_EXECUTABLE = out/server

all: $(SERVER_EXECUTABLE)

$(SERVER_EXECUTABLE): objects
	gcc $(wildcard bin/*.o) -o $(SERVER_EXECUTABLE)

.PHONY: objects
objects:
	$(MAKE) -C src