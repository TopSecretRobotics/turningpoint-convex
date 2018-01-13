// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*
 * autonomous/timer.h
 */

#ifndef AUTONOMOUS_TIMER_H_

#define AUTONOMOUS_TIMER_H_

#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header
#include "vex.h" // vex library header

#include <math.h>
#include <stdlib.h>

/* Timer types and variables */

typedef struct autonomousTimer_s {
    unsigned long actual;
    unsigned long target;
} autonomousTimer_t;

static autonomousTimer_t autonomousTimer = {.actual = 0, .target = 0};

#ifdef __cplusplus
extern "C" {
#endif

/* Timer function declarations */

static void timerSetTimeout(unsigned long target);
static bool timerIsActive(void);
static void timerTick(void);

#define timerRun(TARGET, CODE)                                                                                                     \
    do {                                                                                                                           \
        timerSetTimeout(TARGET);                                                                                                   \
        while (timerIsActive()) {                                                                                                  \
            do                                                                                                                     \
                CODE while (0);                                                                                                    \
            timerTick();                                                                                                           \
        }                                                                                                                          \
    } while (0)

/* Timer function definitions */

inline void
timerSetTimeout(unsigned long target)
{
    autonomousTimer.actual = chTimeNow();
    autonomousTimer.target = autonomousTimer.actual + target;
    return;
}

inline bool
timerIsActive(void)
{
    return (bool)(autonomousTimer.actual <= autonomousTimer.target);
}

inline void
timerTick(void)
{
    vexSleep(25);
    autonomousTimer.actual = chTimeNow();
    return;
}

#ifdef __cplusplus
}
#endif

#endif
