CC := gcc
CFLAGS := `sdl2-config --libs --cflags` -Wall -Wno-incompatible-function-pointer-types -Wno-incompatible-pointer-types -ferror-limit=2000

all: build video test

build: gb.c cpu.c mem.c video.c
	$(CC) $(CFLAGS) -O2 -DNDEBUG -o gb -DGB_MAIN $^

debug: gb.c cpu.c mem.c video.c
	$(CC) $(CFLAGS) -g -o gb -DGB_MAIN $^

profile: gb.c cpu.c mem.c video.c
	`brew --prefix llvm`/bin/clang $(CFLAGS) -DGB_MAIN -g -fprofile-instr-generate -fcoverage-mapping -o gb $^

test: cpu_test.c gb.c mem.c video.c
	$(CC) $(CFLAGS) -g -o cpu_test $^
	./cpu_test

clean:
	rm -f gb cpu_test video video_test

video: video_test.c mem.c gb.c cpu.c
	$(CC) $(CFLAGS) -g -o video_test $^
	./video_test

audio: _audio_test.c
	$(CC) $(CFLAGS) _audio_test.c -o _audio_test
