CC=cc
CFLAGS= -g -Wall -Werror -Wextra -pedantic -Wno-gnu-folding-constant
DEPS = sarah2.h
OBJ = sarah2.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

sarah2: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm $(OBJ) sarah2