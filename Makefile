all: build video test

build: gb.c
	gcc -o gb -DGB_MAIN gb.c

test: gb_test.c
	gcc -o gb_test gb_test.c
	./gb_test

clean:
	rm -f gb gb_test video

VIDEO_CFLAGS := `sdl2-config --libs --cflags` -lSDL2_image 
video: video.c
	gcc $(VIDEO_CFLAGS) -o video video.c && ./video
