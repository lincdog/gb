all: build video test

build: gb.c cpu.c
	gcc -o gb -DGB_MAIN gb.c cpu.c

test: cpu_test.c
	gcc -o cpu_test cpu_test.c
	./cpu_test

clean:
	rm -f gb cpu_test video

VIDEO_CFLAGS := `sdl2-config --libs --cflags` -lSDL2_image 
video: video.c
	gcc $(VIDEO_CFLAGS) -o video video.c && ./video
