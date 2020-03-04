export BIN = $(shell pwd)/bin
SERVER_EXECUTABLE = out/server
SERVER_ROOT = src/server

all: $(SERVER_EXECUTABLE)

$(SERVER_EXECUTABLE): objects
	gcc $(wildcard bin/*.o) -o $(SERVER_EXECUTABLE)

.PHONY: objects
objects:
	$(MAKE) -C $(SERVER_ROOT)

.PHONY: runtest
runtest:
	gcc -I include $(UNIT) -o test/$(subst .,_,$(subst /,_,$(UNIT))).o
	./test/$(subst .,_,$(subst /,_,$(UNIT))).o