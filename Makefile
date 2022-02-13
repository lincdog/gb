CC := gcc
CFLAGS := `sdl2-config --libs --cflags` -Wall -Wno-incompatible-function-pointer-types -ferror-limit=2000

all: build video test

build: gb.c cpu.c mem.c video.c
	$(CC) $(CFLAGS) -o gb -DGB_MAIN gb.c cpu.c mem.c video.c

debug: gb.c cpu.c mem.c video.c
	$(CC) $(CFLAGS) -g -o gb -DGB_MAIN gb.c cpu.c mem.c video.c

test: cpu_test.c gb.c mem.c video.c
	$(CC) $(CFLAGS) -g -o cpu_test cpu_test.c gb.c mem.c video.c
	./cpu_test

clean:
	rm -f gb cpu_test video video_test

video: video_test.c mem.c gb.c cpu.c
	gcc $(CFLAGS) -g -o video_test video_test.c gb.c mem.c cpu.c && ./video_test
