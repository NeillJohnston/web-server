INPUT = main.c
OUTPUT = main.o
SUB = 

all: $(OUTPUT)

$(OUTPUT): $(INPUT) $(SUB)
	gcc -c $(INPUT) -o $(BIN)/$(OUTPUT)

.PHONY: $(SUB)
$(SUB):
	$(MAKE) -C $@