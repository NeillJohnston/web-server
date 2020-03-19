export BIN = $(shell pwd)/bin
export INCLUDE = $(shell pwd)/include
export FLAGS = -Wall -Werror
SERVER_EXECUTABLE = out/server
SERVER_ROOT = src/server

all: $(SERVER_EXECUTABLE)

$(SERVER_EXECUTABLE): server_objects
	gcc $(FLAGS) $(shell ls bin/*.o) -o $(SERVER_EXECUTABLE)

.PHONY: server_objects
server_objects:
	$(MAKE) -C $(SERVER_ROOT)


# --- Unit Testing ---
ALLTESTS = $(shell find -wholename "*/_/test_*.c")
VERBOSE = 2

# Make sure UNIT= is declared when running make runtest
.PHONY: runtest
runtest:
	gcc $(FLAGS) -g -I include -D_VERBOSE=$(VERBOSE) $(UNIT) -o test/$(subst .,_,$(subst /,_,$(UNIT))).o
	./test/$(subst .,_,$(subst /,_,$(UNIT))).o

.PHONY: runalltests
runalltests: $(ALLTESTS)

# Force _VERBOSE to 1 to declutter output
.PHONY: $(ALLTESTS)
$(ALLTESTS):
	gcc $(FLAGS) -I include -D_VERBOSE=1 $@ -o test/$(subst .,_,$(subst /,_,$@)).o
	./test/$(subst .,_,$(subst /,_,$@)).o
