// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*
 * arm.h
 */

#ifndef ARM_H_

#define ARM_H_

#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header
#include "vex.h" // vex library header

#include "pidlib.h"
#include "smartmotor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct arm_s {
    tVexMotor motor;
    bool locked;
} arm_t;

extern arm_t *armGetPtr(void);
extern void armSetup(tVexMotor motor);
extern void armInit(void);
extern void armStart(void);
extern void armMove(int16_t cmd, bool immediate);
extern void armLock(void);
extern void armUnlock(void);

#ifdef __cplusplus
}
#endif

#endif
