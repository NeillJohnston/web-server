export BIN = $(shell pwd)/bin
export INCLUDE = $(shell pwd)/include
export FLAGS = -Wall -Werror
SERVER_EXECUTABLE = out/server
SERVER_ROOT = src/server
SQLITE_SHELL_EXECUTABLE = out/sqlite_shell

SQLITE_LINKS = -ldl -lpthread

all: $(SERVER_EXECUTABLE)

$(SERVER_EXECUTABLE): server_objects sqlite
	gcc $(FLAGS) $(shell ls bin/*.o) -o $(SERVER_EXECUTABLE) $(SQLITE_LINKS)

.PHONY: server_objects
server_objects:
	$(MAKE) -C $(SERVER_ROOT)

.PHONY: sqlite
sqlite: $(BIN)/sqlite3.o $(SQLITE_SHELL_EXECUTABLE)

$(BIN)/sqlite3.o:
	gcc -c sqlite-amalgamation-3310100/sqlite3.c -o $(BIN)/sqlite3.o

$(SQLITE_SHELL_EXECUTABLE):
	gcc sqlite-amalgamation-3310100/shell.c sqlite-amalgamation-3310100/sqlite3.c -lpthread -ldl -o $(SQLITE_SHELL_EXECUTABLE)

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
