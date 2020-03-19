OBJECTS = site.o

all: $(OBJECTS) $(SUB)

$(OBJECTS): $(OBJECTS:.o=.c)
	gcc $(FLAGS) -c -I $(INCLUDE) $(basename $@).c -o $(BIN)/$@