#include "base.h"
#include "cpu.h"
#include "mem.h"
#include "video.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


GBState *initialize_gb(MemInitFlag flag) {
    GBState *state = malloc(sizeof(GBState));
    state->cpu = initialize_cpu();
    state->counter = 0;

    state->ppu = initialize_ppu();
    state->mem = initialize_memory(flag);
    state->timer = initialize_timer();

    /*
        * Load the boot ROM and the cartridge header at least
        * Need info from the header to set up memory (0x147 - cartridge type,
          0x148 - ROM size, 0x149 - RAM size)
    */
    return state;
}

void teardown_gb(GBState *state) {
    teardown_cpu(state->cpu);
    teardown_memory(state->mem);
    teardown_ppu(state->ppu);
    teardown_timer(state->timer);
    free(state);
}

int _dummy(void) { return 1; }

GBTask gb_tasks[] = {
    {.period=256,
    .run_task=&task_div_timer
    },
    {.period=16,
    .run_task=&task_tima_timer
    },
    /*{.period=1,
    .run_task=&task_ppu_cycle
    },
    {.period=4,
    .run_task=&task_dma_cycle
    },
    {.period=4,
    .run_task=&task_interrupt_cycle
    },*/
    {.period=4,
    .run_task=&task_cpu_m_cycle
    },
    {.period=0,
    .run_task=NULL
    }
};

void main_loop(GBState *state, int n_cycles) {
    GBTask task;
    int t = 0;

    while ((n_cycles < 0) || (state->counter < n_cycles)) {
        t = 0;
        task = gb_tasks[0];
        while (task.period != 0) {

            if ((state->counter % task.period) == 0) {
                (*task.run_task)(state);
            }
            t++;
            task = gb_tasks[t];
        }

        state->counter++;
    }
}

CartridgeHeader *read_cart_header(FILE *fp) {
    BYTE *_header = malloc(sizeof(CartridgeHeader));
    if (_header == NULL) {
        printf("Error allocating header\n");
        return NULL;
    }
    memset(_header, 0, sizeof(CartridgeHeader));

    if (fseek(fp, 0x100, SEEK_SET) != 0) {
        printf("Error seeking to 0x100 to read header\n");
        free(_header);
        return NULL;
    }
    int n_read = fread(_header, 1, sizeof(CartridgeHeader), fp);
    if (n_read != sizeof(CartridgeHeader)) {
        printf("Read wrong number %d of bytes\n", n_read);
        free(_header);
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        printf("Error seeking back to 0 after read header\n");
        free(_header);
        return NULL;
    }

    CartridgeHeader *header = (CartridgeHeader *)_header;
    return header;
    
}

#ifdef GB_MAIN

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: gb <ROM file>\n");
        exit(2);
    }
    GBState *state;
    
    FILE *fp;
    fp = fopen(argv[1], "r");
    int n_read = 0;

    if (fp == NULL) {
        printf("Error opening %s\n", argv[1]);
        exit(1);
    }

    CartridgeHeader *header = read_cart_header(fp);

    if (header->cartridge_type == 0x00 &&
        header->rom_size == 0 &&
        header->rom_size == 0) {
            state = initialize_gb(BASIC);
            n_read = fread(
                ((BasicCartState *)state->mem->cartridge->state)->rom, 
                1, 
                0x8000, 
                fp
            );
            if (n_read != 0x8000) {
                printf("Error: ready %04x rather than 0x8000\n", n_read);
            }
    } else {
        printf("Currently unsupported cart type %02x, rom type %02x, ram type %02x\n",
            header->cartridge_type, header->rom_size, header->ram_size);
        
        fclose(fp);
        exit(1);
    }
    
    fclose(fp);

    print_state_info(state, 1);

    main_loop(state, -1);
    
    teardown_gb(state);
    return 0;
}

#endif