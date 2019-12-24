all: test.o

test.o: src/test.c src/obj.c src/str.c src/hash.c
	cc -g -Wall -Wextra -Werror -Isrc src/test.c src/obj.c src/str.c src/hash.c -o test.o