OBJECTS = main.o server.o worker.o config.o
SUB = string http files

all: $(OBJECTS) $(SUB)

$(OBJECTS): $(OBJECTS:.o=.c)
	gcc $(WARNINGS) -c -I $(INCLUDE) $(basename $@).c -o $(BIN)/$@

.PHONY: $(SUB)
$(SUB):
	$(MAKE) -C $@