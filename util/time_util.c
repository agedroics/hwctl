#include <time_util.h>

#define MILLION 1000000
#define BILLION 1000000000

void time_from_millis(struct timespec *tp, long millis) {
    if (millis < 0) {
        tp->tv_sec = (millis + 1) / 1000 - 1;
        tp->tv_nsec = BILLION - (-millis % 1000) * MILLION;
    } else {
        tp->tv_sec = millis / 1000;
        tp->tv_nsec = (millis % 1000) * MILLION;
    }
}

struct timespec time_nanos() {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp;
}

struct timespec time_diff(const struct timespec *tp1, const struct timespec *tp2) {
    struct timespec diff = *tp1;
    time_subtract(&diff, tp2);
    return diff;
}

void time_add(struct timespec *tp, const struct timespec *amt) {
    tp->tv_sec += amt->tv_sec;
    tp->tv_nsec += amt->tv_nsec;
    if (tp->tv_nsec >= BILLION) {
        tp->tv_nsec -= BILLION;
        ++tp->tv_sec;
    }
}

void time_subtract(struct timespec *tp, const struct timespec *amt) {
    tp->tv_sec -= amt->tv_sec;
    tp->tv_nsec -= amt->tv_nsec;
    if (tp->tv_nsec < 0) {
        tp->tv_nsec += BILLION;
        --tp->tv_sec;
    }
}

int time_is_positive(const struct timespec *tp) {
    return tp->tv_sec == 0 && tp->tv_nsec > 0 || tp->tv_sec > 0;
}