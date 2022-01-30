CC := gcc
CFLAGS := `sdl2-config --libs --cflags` -Wall -Wno-incompatible-function-pointer-types -ferror-limit=2000

all: build video test

build: gb.c cpu.c mem.c video.c
	$(CC) $(CFLAGS) -o gb -DGB_MAIN gb.c cpu.c mem.c video.c

debug: gb.c cpu.c mem.c video.c
	$(CC) $(CFLAGS) -g -o gb -DGB_MAIN gb.c cpu.c mem.c video.c

test: cpu_test.c gb.c mem.c
	$(CC) $(CFLAGS) -g -o cpu_test cpu_test.c gb.c mem.c
	./cpu_test

clean:
	rm -f gb cpu_test video

VIDEO_CFLAGS := `sdl2-config --libs --cflags` -lSDL2_image -DGB_VIDEO_MAIN
video: video.c
	gcc $(VIDEO_CFLAGS) -o video video.c && ./video
