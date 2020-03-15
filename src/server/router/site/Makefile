OBJECTS = site.o

all: $(OBJECTS) $(SUB)

$(OBJECTS): $(OBJECTS:.o=.c)
	gcc $(WARNINGS) -c -I $(INCLUDE) $(basename $@).c -o $(BIN)/$@