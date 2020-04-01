export BIN = $(shell pwd)/bin
export INCLUDE = $(shell pwd)/include
export FLAGS = -Wall -Werror

SERVER_OBJECT = out/server
SERVER_ROOT = src/server

SQLITE_LINKS = -ldl -lpthread
OPENSSL_LINKS = -lssl

all: $(SERVER_OBJECT)

$(SERVER_OBJECT): server_objects sqlite
	gcc $(FLAGS) $(shell ls bin/*.o) -o $(SERVER_OBJECT) $(SQLITE_LINKS) $(OPENSSL_LINKS)

.PHONY: server_objects
server_objects:
	$(MAKE) -C $(SERVER_ROOT)

# --- SQLite ---

SQLITE_SHELL_EXECUTABLE = out/sqlite_shell
SQLITE_ROOT = src/sqlite
SQLITE_OBJECT = sqlite3.o

.PHONY: sqlite
sqlite: $(BIN)/sqlite3.o $(SQLITE_SHELL_EXECUTABLE)

$(BIN)/$(SQLITE_OBJECT):
	gcc -c $(SQLITE_ROOT)/sqlite3.c -o $(BIN)/$(SQLITE_OBJECT)

$(SQLITE_SHELL_EXECUTABLE):
	gcc $(SQLITE_ROOT)/shell.c $(SQLITE_ROOT)/sqlite3.c $(SQLITE_LINKS) -o $(SQLITE_SHELL_EXECUTABLE)

# --- Unit Testing ---

ALLTESTS = $(shell find -wholename "*/_/test_*.c")
VERBOSE = 2

# Make sure UNIT= is declared when running make runtest
.PHONY: runtest
runtest:
	gcc $(FLAGS) -g -I include -D_VERBOSE=$(VERBOSE) $(UNIT) $(BIN)/sqlite3.o $(SQLITE_LINKS) -o test/$(subst .,_,$(subst /,_,$(UNIT))).o
	./test/$(subst .,_,$(subst /,_,$(UNIT))).o

.PHONY: runalltests
runalltests: $(ALLTESTS)

# Set _VERBOSE to 1 to declutter output
.PHONY: $(ALLTESTS)
$(ALLTESTS):
	gcc $(FLAGS) -I include -D_VERBOSE=1 $@ -o test/$(subst .,_,$(subst /,_,$@)).o
	./test/$(subst .,_,$(subst /,_,$@)).o
