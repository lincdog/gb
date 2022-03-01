#include "base.h"
#include "mem.h"
#include "video.h"
#include "video.c"
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <sys/time.h>


#define TEST_TILE_A 4
#define N_TEST_TILES 4 + 26
#define TEST_TILE_SPACE 3
#define test_tile_letter(__c) (TEST_TILE_A + (__c - 'A'))

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
    },
    // space
    {
        0, 0, 0, 0,
        0, 0, 0, 0, 
        0, 0, 0, 0,
        0, 0, 0, 0
    },
    // Begin alphabet

    //A
    {

    0x18, 0x18, 0x24, 0x24,
    0x42, 0x42, 0x7E, 0x7E,
    0x42, 0x42, 0x42, 0x42,
    0x42, 0x42, 0x42, 0x42,
    },
    //B
    {

    0x38, 0x38, 0x24, 0x24,
    0x24, 0x24, 0x38, 0x38,
    0x24, 0x24, 0x24, 0x24,
    0x24, 0x24, 0x38, 0x38,
    },
    //C
    {

    0x78, 0x78, 0x84, 0x84,
    0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80,
    0x84, 0x84, 0x78, 0x78,
    },
    //D
    {

    0x38, 0x38, 0x24, 0x24,
    0x22, 0x22, 0x22, 0x22,
    0x22, 0x22, 0x22, 0x22,
    0x24, 0x24, 0x38, 0x38,
    },
    //E
    {

    0x3C, 0x3C, 0x20, 0x20,
    0x20, 0x20, 0x3C, 0x3C,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x3C, 0x3C,
    },
    //F
    {

    0x7C, 0x7C, 0x40, 0x40,
    0x40, 0x40, 0x7C, 0x7C,
    0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40,
    },
    //G
    {

    0x3C, 0x3C, 0x42, 0x42,
    0x80, 0x80, 0x9E, 0x9E,
    0x92, 0x92, 0x82, 0x82,
    0x82, 0x82, 0x7C, 0x7C,
    },
    //H
    {

    0x24, 0x24, 0x24, 0x24,
    0x24, 0x24, 0x3C, 0x3C,
    0x24, 0x24, 0x24, 0x24,
    0x24, 0x24, 0x24, 0x24,
    },
    //I
    {

    0x3E, 0x3E, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x3E, 0x3E,
    },
    //J
    {

    0x3E, 0x3E, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x48, 0x48,
    0x48, 0x48, 0x30, 0x30,
    },
    //K
    {

    0x44, 0x44, 0x48, 0x48,
    0x50, 0x50, 0x60, 0x60,
    0x60, 0x60, 0x50, 0x50,
    0x48, 0x48, 0x48, 0x48,
    },
    //L
    {

    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x3E, 0x3E,
    },
    //M
    {

    0x00, 0x00, 0x7C, 0x7C,
    0x54, 0x54, 0x54, 0x54,
    0x54, 0x54, 0x54, 0x54,
    0x44, 0x44, 0x44, 0x44,
    },
    //N
    {

    0x42, 0x42, 0x62, 0x62,
    0x52, 0x52, 0x52, 0x52,
    0x4A, 0x4A, 0x4A, 0x4A,
    0x46, 0x46, 0x42, 0x42,
    },
    //O
    {

    0x3C, 0x3C, 0x24, 0x24,
    0x24, 0x24, 0x24, 0x24,
    0x24, 0x24, 0x24, 0x24,
    0x24, 0x24, 0x3C, 0x3C,
    },
    //P
    {

    0x70, 0x70, 0x48, 0x48,
    0x48, 0x48, 0x70, 0x70,
    0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40,
    },
    //Q
    {

    0x18, 0x18, 0x24, 0x24,
    0x24, 0x24, 0x24, 0x24,
    0x24, 0x24, 0x24, 0x24,
    0x18, 0x18, 0x06, 0x06,
    },
    //R
    {

    0x70, 0x70, 0x48, 0x48,
    0x48, 0x48, 0x50, 0x50,
    0x60, 0x60, 0x50, 0x50,
    0x48, 0x48, 0x44, 0x44,
    },
    //S
    {

    0x1C, 0x1C, 0x20, 0x20,
    0x20, 0x20, 0x18, 0x18,
    0x04, 0x04, 0x04, 0x04,
    0x38, 0x38, 0x00, 0x00,
    },
    //T
    {

    0x3E, 0x3E, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08,
    },
    //U
    {

    0x44, 0x44, 0x44, 0x44,
    0x44, 0x44, 0x44, 0x44,
    0x44, 0x44, 0x44, 0x44,
    0x44, 0x44, 0x38, 0x38,
    },
    //V
    {

    0x00, 0x00, 0x41, 0x41,
    0x41, 0x41, 0x22, 0x22,
    0x22, 0x22, 0x14, 0x14,
    0x14, 0x14, 0x08, 0x08,
    },
    //W
    {

    0x00, 0x00, 0x82, 0x82,
    0x82, 0x82, 0x92, 0x92,
    0x54, 0x54, 0x54, 0x54,
    0x28, 0x28, 0x00, 0x00,
    },
    //X
    {

    0x44, 0x44, 0x44, 0x44,
    0x28, 0x28, 0x28, 0x28,
    0x10, 0x10, 0x28, 0x28,
    0x28, 0x28, 0x44, 0x44,
    },
    //Y
    {

    0x44, 0x44, 0x44, 0x44,
    0x28, 0x28, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10,
    },
    //Z
    {

    0xFE, 0xFE, 0x02, 0x02,
    0x04, 0x04, 0x08, 0x08,
    0x10, 0x10, 0x20, 0x20,
    0x40, 0x40, 0xFE, 0xFE,
    }
};

const BYTE test_tilemap[TILEMAP_SIZE_BYTES] = {
    test_tile_letter('H'), test_tile_letter('E'), test_tile_letter('L'), test_tile_letter('L'), test_tile_letter('O'), TEST_TILE_SPACE, test_tile_letter('W'), test_tile_letter('O'), test_tile_letter('R'), test_tile_letter('L'), test_tile_letter('D'), 2, 0, 1, 2, 0,
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
    },
    // A
    {
        0, 0, 0, 3, 3, 0, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 3, 0, 0, 0, 0, 3, 0,
        0, 3, 3, 3, 3, 3, 3, 0,
        0, 3, 0, 0, 0, 0, 3, 0,
        0, 3, 0, 0, 0, 0, 3, 0,
        0, 3, 0, 0, 0, 0, 3, 0,
        0, 3, 0, 0, 0, 0, 3, 0
    },
    // B
    {
        0, 0, 3, 3, 3, 0, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 3, 3, 0, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 3, 3, 0, 0, 0
    },
    // C
    {
        0, 3, 3, 3, 3, 0, 0, 0,
        3, 0, 0, 0, 0, 3, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 3, 0, 0,
        0, 3, 3, 3, 3, 0, 0, 0
    },
    // D
    {
        0, 0, 3, 3, 3, 0, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 0, 3, 0,
        0, 0, 3, 0, 0, 0, 3, 0,
        0, 0, 3, 0, 0, 0, 3, 0,
        0, 0, 3, 0, 0, 0, 3, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 3, 3, 0, 0, 0
    },
    // E
    {
        0, 0, 3, 3, 3, 3, 0, 0,
        0, 0, 3, 0, 0, 0, 0, 0,
        0, 0, 3, 0, 0, 0, 0, 0,
        0, 0, 3, 3, 3, 3, 0, 0,
        0, 0, 3, 0, 0, 0, 0, 0,
        0, 0, 3, 0, 0, 0, 0, 0,
        0, 0, 3, 0, 0, 0, 0, 0,
        0, 0, 3, 3, 3, 3, 0, 0
    },
    // F
    {
        0, 3, 3, 3, 3, 3, 0, 0,
        0, 3, 0, 0, 0, 0, 0, 0,
        0, 3, 0, 0, 0, 0, 0, 0,
        0, 3, 3, 3, 3, 3, 0, 0,
        0, 3, 0, 0, 0, 0, 0, 0,
        0, 3, 0, 0, 0, 0, 0, 0,
        0, 3, 0, 0, 0, 0, 0, 0,
        0, 3, 0, 0, 0, 0, 0, 0
    },
    // G
    {
        0, 0, 3, 3, 3, 3, 0, 0,
        0, 3, 0, 0, 0, 0, 3, 0,
        3, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 3, 3, 3, 3, 0,
        3, 0, 0, 3, 0, 0, 3, 0,
        3, 0, 0, 0, 0, 0, 3, 0,
        3, 0, 0, 0, 0, 0, 3, 0,
        0, 3, 3, 3, 3, 3, 0, 0
    },
    // H
    {
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 3, 3, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0
    },
    // I
    {
        0, 0, 3, 3, 3, 3, 3, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 3, 3, 3, 3, 3, 0
    },
    // J
    {
        0, 0, 3, 3, 3, 3, 3, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 3, 0, 0, 3, 0, 0, 0,
        0, 3, 0, 0, 3, 0, 0, 0,
        0, 0, 3, 3, 0, 0, 0, 0
    },
    // K
    {
        0, 3, 0, 0, 0, 3, 0, 0,
        0, 3, 0, 0, 3, 0, 0, 0,
        0, 3, 0, 3, 0, 0, 0, 0,
        0, 3, 3, 0, 0, 0, 0, 0,
        0, 3, 3, 0, 0, 0, 0, 0,
        0, 3, 0, 3, 0, 0, 0, 0,
        0, 3, 0, 0, 3, 0, 0, 0,
        0, 3, 0, 0, 3, 0, 0, 0
    },
    // L
    {
        0, 0, 3, 0, 0, 0, 0, 0,
        0, 0, 3, 0, 0, 0, 0, 0,
        0, 0, 3, 0, 0, 0, 0, 0,
        0, 0, 3, 0, 0, 0, 0, 0,
        0, 0, 3, 0, 0, 0, 0, 0,
        0, 0, 3, 0, 0, 0, 0, 0,
        0, 0, 3, 0, 0, 0, 0, 0,
        0, 0, 3, 3, 3, 3, 3, 0
    },
    // M
    {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 3, 3, 3, 3, 3, 0, 0,
        0, 3, 0, 3, 0, 3, 0, 0,
        0, 3, 0, 3, 0, 3, 0, 0,
        0, 3, 0, 3, 0, 3, 0, 0,
        0, 3, 0, 3, 0, 3, 0, 0,
        0, 3, 0, 0, 0, 3, 0, 0,
        0, 3, 0, 0, 0, 3, 0, 0
    },
    // N
    {
        0, 3, 0, 0, 0, 0, 3, 0,
        0, 3, 3, 0, 0, 0, 3, 0,
        0, 3, 0, 3, 0, 0, 3, 0,
        0, 3, 0, 3, 0, 0, 3, 0,
        0, 3, 0, 0, 3, 0, 3, 0,
        0, 3, 0, 0, 3, 0, 3, 0,
        0, 3, 0, 0, 0, 3, 3, 0,
        0, 3, 0, 0, 0, 0, 3, 0
    },
    // O
    {
        0, 0, 3, 3, 3, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 3, 3, 3, 0, 0
    },
    // P
    {
        0, 3, 3, 3, 0, 0, 0, 0,
        0, 3, 0, 0, 3, 0, 0, 0,
        0, 3, 0, 0, 3, 0, 0, 0,
        0, 3, 3, 3, 0, 0, 0, 0,
        0, 3, 0, 0, 0, 0, 0, 0,
        0, 3, 0, 0, 0, 0, 0, 0,
        0, 3, 0, 0, 0, 0, 0, 0,
        0, 3, 0, 0, 0, 0, 0, 0
    },
    // Q
    {
        0, 0, 0, 3, 3, 0, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 0, 3, 0, 0,
        0, 0, 0, 3, 3, 0, 0, 0,
        0, 0, 0, 0, 0, 3, 3, 0
    },
    // R
    {
        0, 3, 3, 3, 0, 0, 0, 0,
        0, 3, 0, 0, 3, 0, 0, 0,
        0, 3, 0, 0, 3, 0, 0, 0,
        0, 3, 0, 3, 0, 0, 0, 0,
        0, 3, 3, 0, 0, 0, 0, 0,
        0, 3, 0, 3, 0, 0, 0, 0,
        0, 3, 0, 0, 3, 0, 0, 0,
        0, 3, 0, 0, 0, 3, 0, 0
    },
    // S
    {
        0, 0, 0, 3, 3, 3, 0, 0,
        0, 0, 3, 0, 0, 0, 0, 0,
        0, 0, 3, 0, 0, 0, 0, 0,
        0, 0, 0, 3, 3, 0, 0, 0,
        0, 0, 0, 0, 0, 3, 0, 0,
        0, 0, 0, 0, 0, 3, 0, 0,
        0, 0, 3, 3, 3, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    },
    // T
    {
        0, 0, 3, 3, 3, 3, 3, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0
    },
    // U
    {
        0, 3, 0, 0, 0, 3, 0, 0,
        0, 3, 0, 0, 0, 3, 0, 0,
        0, 3, 0, 0, 0, 3, 0, 0,
        0, 3, 0, 0, 0, 3, 0, 0,
        0, 3, 0, 0, 0, 3, 0, 0,
        0, 3, 0, 0, 0, 3, 0, 0,
        0, 3, 0, 0, 0, 3, 0, 0,
        0, 0, 3, 3, 3, 0, 0, 0
    },
    // V
    {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 3, 0, 0, 0, 0, 0, 3,
        0, 3, 0, 0, 0, 0, 0, 3,
        0, 0, 3, 0, 0, 0, 3, 0,
        0, 0, 3, 0, 0, 0, 3, 0,
        0, 0, 0, 3, 0, 3, 0, 0,
        0, 0, 0, 3, 0, 3, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0
    },
    // W
    {
        0, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 3, 0,
        3, 0, 0, 0, 0, 0, 3, 0,
        3, 0, 0, 3, 0, 0, 3, 0,
        0, 3, 0, 3, 0, 3, 0, 0,
        0, 3, 0, 3, 0, 3, 0, 0,
        0, 0, 3, 0, 3, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    },
    // X
    {
        0, 3, 0, 0, 0, 3, 0, 0,
        0, 3, 0, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 3, 0, 0, 0,
        0, 0, 3, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0, 0,
        0, 0, 3, 0, 3, 0, 0, 0,
        0, 0, 3, 0, 3, 0, 0, 0,
        0, 3, 0, 0, 0, 3, 0, 0
    },
    // Y
    {
        0, 3, 0, 0, 0, 3, 0, 0,
        0, 3, 0, 0, 0, 3, 0, 0,
        0, 0, 3, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0, 0
    },
    // Z
    {
        3, 3, 3, 3, 3, 3, 3, 0,
        0, 0, 0, 0, 0, 0, 3, 0,
        0, 0, 0, 0, 0, 3, 0, 0,
        0, 0, 0, 0, 3, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0, 0,
        0, 0, 3, 0, 0, 0, 0, 0,
        0, 3, 0, 0, 0, 0, 0, 0,
        3, 3, 3, 3, 3, 3, 3, 0
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

    ppu->misc.scx = 0;
    ppu->misc.scy = 0;
    ppu->misc.wx = 7;
    ppu->misc.wy = 10;
    ppu->misc.bgp = PALETTE_DEFAULT;
    ppu->misc.obp0 = 0b11001100;
    ppu->misc.obp1 = 0b00110011;

    ppu->stat.mode = VBLANK;

    BYTE *mem = vtstate->mem;
    /* Set up tile data at area 1 */

    memset(&mem[0x8000], 0xFF, 0x1000);

    for (int i = 0; i < N_TEST_TILES; i++) {
        memcpy(
            &mem[0x8000 + TILE_SIZE_BYTES*i],
            &test_tiles_packed[i],
            TILE_SIZE_BYTES
        );
    }


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

    for (int i = TEST_TILE_A; i < N_TEST_TILES; i++) {
        printf("//%c\n{\n", 'A' + (i - TEST_TILE_A));
        print_packed(test_tiles_unpacked[i]);
        printf("\n},\n");
    }
    
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
