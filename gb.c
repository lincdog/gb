#include "base.h"
#include "cpu.h"
#include "mem.h"
#include "video.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


GBState *initialize_gb(BYTE flags) {
    GBState *state = malloc(sizeof(GBState));
    state->cpu = initialize_cpu();
    state->counter = 0;

    state->ppu = initialize_ppu();
    state->mem = initialize_memory(0);
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
    teardown_memory(state->mem, 0);
    teardown_ppu(state->ppu);
    teardown_timer(state->timer);
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
    /*{.period=1,
    .run_task=&ppu_cycle
    },
    {.period=4,
    .run_task=&dma_cycle
    },
    {.period=4,
    .run_task=&interrupt_cycle
    },*/
    {.period=4,
    .run_task=&cpu_m_cycle
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

#ifdef GB_MAIN

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: gb <ROM file>\n");
        exit(2);
    }

    GBState *state = initialize_gb(0);
    
    FILE *fp;
    fp = fopen(argv[1], "r");

    if (fp == NULL) {
        printf("Error opening %s\n", argv[1]);
        exit(1);
    }

    BYTE *header = malloc(sizeof(CartridgeHeader));
    if (header == NULL) {
        printf("Error allocating header\n");
        fclose(fp);
        exit(1);
    }
    memset(header, 0, sizeof(CartridgeHeader));

    if (fseek(fp, 0x100, SEEK_SET) != 0) {
        printf("Error seeking to 0x100 to read header\n");
        fclose(fp);
        exit(1);
    }

    int n_read = fread(header, 1, sizeof(CartridgeHeader), fp);
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