#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#define TIMER_CLOCK CLOCK_MONOTONIC

uint64_t timespec_to_usec(struct timespec t) {
    return 1000000 * (uint64_t)t.tv_sec + (uint64_t)t.tv_nsec / 1000;
}

void delay(uint64_t time_us) {
    struct timespec a;
    struct timespec b;

    uint64_t diff = 0;

    clock_gettime(TIMER_CLOCK, &a);
    uint64_t init_usec = timespec_to_usec(a);

    //printf("Starting frame %d, nsec is %ld", looper, a.tv_nsec);

    do {
            clock_gettime(TIMER_CLOCK, &b);
            diff = timespec_to_usec(b) - init_usec;
    } while (diff < time_us);
}

 void signalHandler(int sig) {
    printf("Caught SIGALRM\n");
    fflush(stdout);
}

int main(void) {
    int pid = fork();

    if (pid == 0) {
        while (1) {
            delay(16666);
            kill(0, SIGALRM);
        }
        
    } else {
        // parent

        while (1) {
            signal(SIGALRM, signalHandler); 

        }
       //sleep(10);

    }
}