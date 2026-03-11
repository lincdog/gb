#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define TIMER_CLOCK CLOCK_MONOTONIC

uint64_t timespec_to_nsec(struct timespec t) {
    return 1000000000 * (uint64_t)t.tv_sec + (uint64_t)t.tv_nsec;
}

void delay(uint64_t time_ns) {
    struct timespec a;
    struct timespec b;

    uint64_t diff = 0;

    clock_gettime(TIMER_CLOCK, &a);
    uint64_t init_nsec = timespec_to_nsec (a);

    //printf("Starting frame %d, nsec is %ld", looper, a.tv_nsec);

    do {
            clock_gettime(TIMER_CLOCK, &b);
            diff = timespec_to_nsec(b) - init_nsec;
    } while (diff < time_ns);
}

void delay_for_frames(long n_frames, uint64_t time_ns) {
    long frame;
    for (frame=0; frame < n_frames; frame++) {
        delay(time_ns);
    } 

    //return NULL;
}

void *pthread_delay(void *) {
    //usleep(1000000);
    delay_for_frames(60, 16666000);
}

int n = 0;

void *increment(void *foo) {
    n += 1;
}

int main(void) {
    pthread_t tid_inc;

    for (int cycles = 0; cycles < 1; cycles++) {
        for (int i = 0; i < 4; i++) {
            pthread_create(&tid_inc, NULL, pthread_delay, NULL);
            pthread_join(tid_inc, NULL);
            n++;
        }
    }

    printf("N = %d", n);
}