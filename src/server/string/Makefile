OBJECTS = streamed_string.o bounded_string.o

all: $(OBJECTS)

$(OBJECTS): $(OBJECTS:.o=.c)
	gcc $(FLAGS) -c -I $(INCLUDE) $(basename $@).c -o $(BIN)/$@