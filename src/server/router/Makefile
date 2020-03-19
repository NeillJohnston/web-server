OBJECTS = router.o
SUB = site static dynamic

all: $(OBJECTS) $(SUB)

$(OBJECTS): $(OBJECTS:.o=.c)
	gcc $(FLAGS) -c -I $(INCLUDE) $(basename $@).c -o $(BIN)/$@

.PHONY: $(SUB)
$(SUB):
	$(MAKE) -C $@