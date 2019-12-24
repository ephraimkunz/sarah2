CC= cc
CFLAGS= -Wall -Werror -Wextra -pedantic -Wno-gnu-folding-constant -O3
DEPS = sarah2.h
OBJ = sarah2.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

test: $(OBJ) test.o
	$(CC) -o $@ $^ $(CFLAGS)
	./test

.PHONY: clean

clean:
	rm $(OBJ) sarah2 test test.o