#include "base.h"
#include "mem.h"
#include "video.h"
#include "video.c"
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <sys/time.h>


const BYTE test_tiles_packed[][16] = {
    {
        0xFF, 0x00, 0x7E, 0xFF, 
        0x85, 0x81, 0x89, 0x83,
        0x93, 0x85, 0xA5, 0x8B, 
        0xC9, 0x97, 0x7E, 0xFF
    },
    {
        0x55, 0x33, 0x55, 0x33,
        0x55, 0x33, 0x55, 0x33,
        0x55, 0x33, 0x55, 0x33,
        0x55, 0x33, 0x55, 0x33
    },
    {
        0xF0, 0xF0, 0x00, 0xF0,
        0xF0, 0x00, 0x00, 0x00,
        0xF0, 0x00, 0x00, 0xF0,
        0xF0, 0xF0, 0x00, 0x00
    }
};

const BYTE test_tilemap[TILEMAP_SIZE_BYTES] = {
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0,
    1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1,
    2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0
};

const BYTE test_tiles_unpacked[][64] = {
    {
        1, 1, 1, 1, 1, 1, 1, 1,
        2, 3, 3, 3, 3, 3, 3, 2,
        3, 0, 0, 0, 0, 1, 0, 3,
        3, 0, 0, 0, 1, 0, 2, 3,
        3, 0, 0, 1, 0, 2, 1, 3,
        3, 0, 1, 0, 2, 1, 2, 3,
        3, 1, 0, 2, 1, 2, 2, 3,
        2, 3, 3, 3, 3, 3, 3, 2
    },
    {
        0, 1, 2, 3, 0, 1, 2, 3,
        0, 1, 2, 3, 0, 1, 2, 3,
        0, 1, 2, 3, 0, 1, 2, 3,
        0, 1, 2, 3, 0, 1, 2, 3,
        0, 1, 2, 3, 0, 1, 2, 3,
        0, 1, 2, 3, 0, 1, 2, 3,
        0, 1, 2, 3, 0, 1, 2, 3,
        0, 1, 2, 3, 0, 1, 2, 3,
    },
    {
        3, 3, 3, 3, 0, 0, 0, 0,
        2, 2, 2, 2, 0, 0, 0, 0,
        1, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 0, 0, 0, 0,
        2, 2, 2, 2, 0, 0, 0, 0,
        3, 3, 3, 3, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    }
};

typedef struct {
    GBState *state;
    BYTE *mem;
    BYTE *final_pixels;
} VideoTestState;

VideoTestState *initialize_video_tests(GBState *state) {
    VideoTestState *vtstate = malloc(sizeof(VideoTestState));
    vtstate->state = state;
    vtstate->mem = ((DebugMemState *)state->mem->system->state)->mem;
    
    vtstate->final_pixels = (BYTE *)malloc(GB_HEIGHT_PX * GB_WIDTH_PX);
    memset(vtstate->final_pixels, 0, GB_HEIGHT_PX*GB_WIDTH_PX);

    return vtstate;
}

void setup_test(VideoTestState *vtstate) {
    GBState *state = vtstate->state;
    PPUState *ppu = state->ppu;
    /* Set LCDC to known state */
    ppu->lcdc.lcd_enable = ON;
    ppu->lcdc.win_map_area = MAP_AREA0;
    ppu->lcdc.window_enable = OFF;
    ppu->lcdc.bg_win_data_area = DATA_AREA1;
    ppu->lcdc.bg_map_area = MAP_AREA0;
    ppu->lcdc.obj_size = OBJ_8x8;
    ppu->lcdc.obj_enable = OFF;
    ppu->lcdc.bg_window_enable = ON;

    ppu->misc.scx = 100;
    ppu->misc.scy = 100;
    ppu->misc.wx = 7;
    ppu->misc.wy = 10;
    ppu->misc.bgp = PALETTE_DEFAULT;
    ppu->misc.obp0 = 0b11001100;
    ppu->misc.obp1 = 0b00110011;

    ppu->stat.mode = VBLANK;

    BYTE *mem = vtstate->mem;
    /* Set up tile data at area 1 */
    memset(&mem[0x8000], 0xFF, 0x1000);
    memcpy(&mem[0x8000], &test_tiles_packed[0], 16);
    memcpy(&mem[0x8010], &test_tiles_packed[1], 16);
    memcpy(&mem[0x8020], &test_tiles_packed[2], 16);


    /* Set up tile map at area 0 */
    memcpy(&mem[0x9800], &test_tilemap, 32*32);

    /* Set up a few OAM entries */
    OAMEntry *oam_table = &mem[OAM_BASE];
    oam_table[0].x = 8;
    oam_table[0].y = 16;
    oam_table[0].index = 0;
    oam_table[0].flags = 0xF0;

    oam_table[1].x = 16;
    oam_table[1].y = 106;
    oam_table[1].index = 1;
    oam_table[1].flags = 0x00;
}

void run_test(VideoTestState *vtstate) {
    for (int i = 0; i < (2*PPU_PER_FRAME); i++)
        task_ppu_cycle(vtstate->state);
}

void teardown_video_tests(VideoTestState *vtstate) {
    free(vtstate->final_pixels);
    /*SDL_DestroyTexture(vtstate->gb_texture);
    SDL_FreeSurface(vtstate->gb_surface);
    SDL_DestroyRenderer(vtstate->renderer);
    SDL_DestroyWindow(vtstate->window);*/
    free(vtstate);
}

int main(int argc, char *argv[]) {

    print_unpacked(test_tiles_packed[0]);
    print_packed(test_tiles_unpacked[0]);
    print_packed(test_tiles_unpacked[1]);
    print_packed(test_tiles_unpacked[2]);

    /*BYTE *test_pixels = malloc(32*32);
    memset(test_pixels, 0, 32*32);
    for (int i = 0; i < 32; i += 8) {
        for (int j = 0; j < 32; j += 8) {
            unpack_tile(&test_tiles_packed[0], test_pixels, 0, i, j, 32);
        }
    }

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            printf("%02X ", test_pixels[32*i + j]);
        }
        printf("\n");
    }*/
    struct timespec preinit_ts, postinit_ts, pretest_ts, posttest_ts;
    clock_gettime(CLOCK_REALTIME, &preinit_ts);
    
    GBState *state = initialize_gb(DEBUG);
    clock_gettime(CLOCK_REALTIME, &postinit_ts);
    
    VideoTestState *vtstate = initialize_video_tests(state);
    clock_gettime(CLOCK_REALTIME, &pretest_ts);

    setup_test(vtstate);
    run_test(vtstate);
    SDL_Event event;
    clock_gettime(CLOCK_REALTIME, &posttest_ts);    
    while (1) {
        SDL_PollEvent(&event);
        if(event.type == SDL_QUIT)
            break;
    }
    
    printf("%lu\n", posttest_ts.tv_nsec - pretest_ts.tv_nsec);
    
    teardown_video_tests(vtstate);
    teardown_gb(state);
        
 
    return 0;
}
