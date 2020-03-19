OBJECTS = main.o server.o worker.o config.o
SUB = string http files router

all: $(OBJECTS) $(SUB)

$(OBJECTS): $(OBJECTS:.o=.c)
	gcc $(FLAGS) -c -I $(INCLUDE) $(basename $@).c -o $(BIN)/$@

.PHONY: $(SUB)
$(SUB):
	$(MAKE) -C $@