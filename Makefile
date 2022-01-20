CC := gcc
CFLAGS := -Wall

all: build video test

build: gb.c cpu.c mem.c
	$(CC) $(CFLAGS) -o gb -DGB_MAIN gb.c cpu.c mem.c

debug: gb.c cpu.c mem.c
	$(CC) $(CFLAGS) -g -o gb -DGB_MAIN gb.c cpu.c mem.c

test: cpu_test.c mem.c
	$(CC) $(CFLAGS) -o cpu_test cpu_test.c mem.c
	./cpu_test

clean:
	rm -f gb cpu_test video

VIDEO_CFLAGS := `sdl2-config --libs --cflags` -lSDL2_image 
video: video.c
	gcc $(VIDEO_CFLAGS) -o video video.c && ./video
