OBJECTS = files.o

all: $(OBJECTS)

$(OBJECTS): $(OBJECTS:.o=.c)
	gcc $(FLAGS) -c -I $(INCLUDE) $(basename $@).c -o $(BIN)/$@