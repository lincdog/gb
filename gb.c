#include "base.h"
#include "cpu.h"
#include "mem.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


GBState *initialize_gb(void) {
    GBState *state = malloc(sizeof(GBState));
    state->cpu = initialize_cpu();
    state->counter = 0;

    /*// TO DELETE
    BYTE *code = malloc(0x10000);
    state->code = memset(code, 0xFF, 0x10000);

    state->io_regs = initialize_ioregs();
    
    memcpy(&state->code[0x104], 
        &GAMEBOY_LOGO, 
        sizeof(GAMEBOY_LOGO));*/
    /*
        * Load the boot ROM and the cartridge header at least
        * Need info from the header to set up memory (0x147 - cartridge type,
          0x148 - ROM size, 0x149 - RAM size)
    */
    return state;
}

void teardown_gb(GBState *state) {
    teardown_cpu(state->cpu);
    //free(state->code);
    free(state);
}

int _dummy(void) { return 1; }

GBTask gb_tasks[] = {
    {.period=256,
    .run_task=&div_timer
    },
    {.period=16,
    .run_task=&tima_timer
    },
    {.period=1,
    .run_task=&ppu_cycle
    },
    {.period=4,
    .run_task=&dma_cycle
    },
    {.period=4,
    .run_task=&interrupt_cycle
    },
    {.period=4,
    .run_task=&cpu_m_cycle
    },
};
#define GB_N_TASKS 5

void main_loop(GBState *state, int n_cycles) {
    
    while ((n_cycles < 0) || (state->counter < n_cycles)) {
        for (int i = 0; i < GB_N_TASKS; i++) {
            if (state->counter % gb_tasks[i].period == 0) {
                *gb_tasks[i].run_task(state);
            }
        }
        if (state->cpu->r.pc > 34) {
            _dummy();
        }

        // check clock % 4; fetch instruction
        // check clock % 4; execute current instruction 
        // run PPU code
        // check interrupts

        state->counter++;
    }
}

#ifdef GB_MAIN

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: gb <ROM file>\n");
        exit(2);
    }

    GBState *state = initialize_gb();
    
    FILE *fp;
    fp = fopen(argv[1], "r");

    if (fp == NULL) {
        printf("Error opening %s\n", argv[1]);
        exit(1);
    }

    BYTE *header = malloc(sizeof(CartridgeHeader));
    if (header == NULL) {
        printf("Error allocating header\n");
        close(fp);
        exit(1);
    }
    memset(header, 0, sizeof(CartridgeHeader));

    if (fseek(fp, 0x100, SEEK_SET) != 0) {
        printf("Error seeking to 0x100 to read header\n");
        close(fp);
        exit(1);
    }

    int n_read = fread(header, 1, 0x100, sizeof(CartridgeHeader));
    printf("n read: %d\n", n_read);
    
    for (int i = 0; i < n_read; i++) {
        if (i % 8 == 0) {
            printf("\n%02x\t", i);
        }
        printf("%02x ", header[i]);
    }
    printf("\n");

    fclose(fp);

    print_state_info(state, 1);

    main_loop(state, -1);
    

    teardown_gb(state);
    return 0;
}

#endif