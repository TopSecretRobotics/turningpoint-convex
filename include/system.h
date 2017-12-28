// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*
 * system.h
 */

#ifndef SYSTEM_H_

#define SYSTEM_H_

#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header
#include "vex.h" // vex library header

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*systemCallback_t)(void);

typedef struct system_s {
    bool enabled;
    systemCallback_t init;
    systemCallback_t start;
    systemCallback_t lock;
    systemCallback_t unlock;
} system_t;

extern void systemInitAll(void);
extern void systemStartAll(void);
extern void systemLockAll(void);
extern void systemUnlockAll(void);

#ifdef __cplusplus
}
#endif

#endif
