OBJECTS = http_request.o http_response.o

all: $(OBJECTS)

$(OBJECTS): $(OBJECTS:.o=.c)
	gcc $(FLAGS) -c -I $(INCLUDE) $(basename $@).c -o $(BIN)/$@