export BIN = $(shell pwd)/bin
export INCLUDE = $(shell pwd)/include
export WARNINGS = -Wall -Werror
SERVER_EXECUTABLE = out/server
SERVER_ROOT = src/server

all: $(SERVER_EXECUTABLE)

$(SERVER_EXECUTABLE): server_objects
	gcc $(WARNINGS) $(shell ls bin/*.o) -o $(SERVER_EXECUTABLE)

.PHONY: server_objects
server_objects:
	$(MAKE) -C $(SERVER_ROOT)

.PHONY: runtest
runtest:
	gcc -I include $(UNIT) -o test/$(subst .,_,$(subst /,_,$(UNIT))).o
	./test/$(subst .,_,$(subst /,_,$(UNIT))).o

.PHONY: runalltests
runalltests: $(shell ls test)

.PHONY: $(shell ls test)
$(shell ls test):
	test/$@