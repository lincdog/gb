#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

void run_target(char *prog) {
    ptrace(PT_TRACE_ME, 0, 0, 0);
    execl(prog, prog, (char*)NULL);
}

void run_emulator(pid_t child) {
    int wait_status;
    struct timespec req;
    unsigned long int count = 1;

    /* set up the emulation speed */
    req.tv_sec = 0;
    req.tv_nsec = 2000;

    /* wait for stop on first instruction */
    wait(&wait_status);
    while (WIFSTOPPED(wait_status)) {
        /* this loop will repeat at every instruction, so it executes the
         * instruction and sleeps for the amount of time needed to 
         * emulate the wanted speed.
         */
        if (ptrace(PT_STEP, child, 0, 0) < 0) {
            perror("ptrace");
            return ;
        }
        wait(&wait_status);

        /* this does the sleep */
        nanosleep(&req, NULL);
    }
}

int main(int argc, char *argv[])
{
    pid_t child;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s [prog_name]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    child = fork();

    if (!child)
        run_target(argv[1]);
    else if (child > 0)
        run_emulator(child);
    else {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    return 0;
}