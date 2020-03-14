OBJECTS = files.o

all: $(OBJECTS)

$(OBJECTS): $(OBJECTS:.o=.c)
	gcc $(WARNINGS) -c -I $(INCLUDE) $(basename $@).c -o $(BIN)/$@