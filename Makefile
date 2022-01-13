all: build test

build: gb.c
	gcc -o gb -DGB_MAIN gb.c

test: gb_test.c
	gcc -o gb_test gb_test.c
	./gb_test

clean:
	rm -f gb gb_test
