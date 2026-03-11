#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
//#include <math.h>
#include <time.h>
//#include <unistd.h>
//#include <sys/time.h>
//#include <sys/select.h>

#define TIMER_CLOCK CLOCK_MONOTONIC

/*
typedef struct {
    struct timespec pre;
    struct timespec split;
    uint64_t diff;
    uint64_t max_usec;
    uint64_t bad_thresh;
    uint64_t n_bad;
    uint64_t total_usec;
    uint64_t n_calls;
} my_timer_t;

void timer_init(my_timer_t *t, uint64_t bad_thresh) {
    t->pre.tv_sec = 0;
    t->pre.tv_nsec = 0;

    t->diff = 0;
    t->max_usec = 0;
    t->total_usec = 0;
    t->n_calls = 0;
    t->bad_thresh = bad_thresh;
    t->n_bad = 0;
}

void timer_begin(my_timer_t *t) {
    //t->pre = clock_gettime_nsec_np(CLOCK_MONOTONIC);
    clock_gettime(TIMER_CLOCK, &(t->pre));
}

void timer_split(my_timer_t *t) {
    clock_gettime(TIMER_CLOCK, &(t->split));

    long diff;

    diff = (t->split.tv_nsec - t->pre.tv_nsec)>>10; // convert to usec

    t->diff = diff;

    t->total_usec += diff;

    if (diff > t->max_usec)
        t->max_usec = diff;
    
    if (diff > t->bad_thresh)
        t->n_bad++;
    
    t->n_calls++;
}*/

//pthread_t tid;

int success;

long double timespec_to_sec(struct timespec t) {
    long double sec = (long double)t.tv_sec;
    long double nsec = (long double)t.tv_nsec;

    return sec + nsec/1000000000;
}

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

void *delay_for_frames(int n_frames, uint64_t time_ns) {
    int frame;
    for (frame=0; frame < n_frames; frame++) {
        delay(time_ns);
    } 

    return NULL;
}

int main(void) {
    time_t pre, post;

    struct timespec res;
    clock_getres(TIMER_CLOCK, &res);
    printf("Clock resolution is %d sec %d nsec\n", res.tv_sec, res.tv_nsec);

    // pthread stuff

    struct {int v1;} shared_var;

    shared_var.v1 = 0;

    pthread_mutex_t shared_mutex;
    pthread_t tid[2];

    // end pthread stuff

    
    struct timespec initial;
    struct timespec total;

    uint64_t frame_time_ns = 16666000;
    long n_frames = 60;

    long double elapsed;

    long frame;

    clock_gettime(TIMER_CLOCK, &initial);

    for (frame=0; frame < n_frames; frame++) {
        delay(frame_time_ns);
    }

    fflush(stdout);

    printf("After delay\n");

    clock_gettime(TIMER_CLOCK, &total);

    printf("Final sec is %ld, nsec is %ldu\n\ninitial sec is %ld, nsec is %ld\n\n", 
        total.tv_sec, total.tv_nsec, initial.tv_sec, initial.tv_nsec);
    
    long double init_sec = timespec_to_sec(initial);
    long double total_sec = timespec_to_sec(total);
    elapsed = total_sec - init_sec;

    printf("Frames: %d\nElapsed: %lf\n(%lf frames per second)\n", 
    frame, elapsed, frame / elapsed);
    
    return 0;
}


/*
On render scanline function, have it loop to delay for correct timing of scanline
scanline is 456 ppu cycles of 2^20 Hz (4 MHz) which is a time of 108.7 us or 108700 ns
slowing down ppu cycle should slow down everything? because ppu cycle is executed every 
main loop iteration; other operations we can approximate as being instantaneous as the 
host computer clock speed is >1000 times faster - or we can delay the ppu by slightly less
to give some time for other operations
*/