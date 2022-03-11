#include "base.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

APUState *initialize_apu(void) {
    APUState *apu = malloc(sizeof(APUState));
    if (apu == NULL) {
        printf("Error allocating APU\n");
        exit(1);
    }
    /*
    FF10: 0x80 1000 0000
    FF11: 0xBF 1011 1111
    FF12: 0xF3 1111 0011
    FF13: 0xFF 1111 1111
    FF14: 0xBF 1011 1111
    FF16: 0x3F 0011 1111
    FF17: 0x00 0000 0000
    FF18: 0xFF 1111 1111
    FF19: 0xBF
    FF1A: 0x7F
    FF1B: 0xFF
    FF1C: 0x9F
    FF1D: 0xFF
    FF1E: 0xBF
    FF20: 0xFF
    FF21: 0
    FF22: 0
    FF23: 0xBF
    FF24: 0x77
    FF25: 0xF3
    FF26: 0xF1

    */

}

void teardown_apu(APUState *apu) {
    free(apu);
}