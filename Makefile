CC := gcc
CFLAGS := `sdl2-config --libs --cflags` -Wall -Wno-incompatible-function-pointer-types -Wno-incompatible-pointer-types -ferror-limit=2000
GBFILES := gb.c cpu.c mem.c video.c

all: build cputest videotest

build: $(GBFILES)
	$(CC) $(CFLAGS) -O2 -DNDEBUG -o gb -DGB_MAIN $^

debug: $(GBFILES)
	$(CC) $(CFLAGS) -g -o gb -DGB_MAIN $^

profile: $(GBFILES)
	`brew --prefix llvm`/bin/clang $(CFLAGS) -DGB_MAIN -g -fprofile-instr-generate -fcoverage-mapping -o gb $^

cputest: cpu_test.c gb.c mem.c video.c
	$(CC) $(CFLAGS) -g -o cpu_test $^
	./cpu_test

videotest: video_test.c mem.c gb.c cpu.c
	$(CC) $(CFLAGS) -g -o video_test $^
	./video_test

audiotest: _audio_test.c
	$(CC) $(CFLAGS) _audio_test.c -o _audio_test

clean:
	rm -f gb cpu_test video video_test _audio_test