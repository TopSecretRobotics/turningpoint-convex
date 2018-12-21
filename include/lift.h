// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*
 * lift.h
 */

#ifndef LIFT_H_

#define LIFT_H_

#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header
#include "vex.h" // vex library header

#include "pidlib.h"
#include "smartmotor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lift_s {
    tVexMotor motor;
    tVexDigitalPin limit;
    bool locked;
} lift_t;

extern lift_t *liftGetPtr(void);
extern void liftSetup(tVexMotor motor, tVexDigitalPin limit);
extern void liftInit(void);
extern void liftStart(void);
extern void liftMove(int16_t cmd, bool immediate);
extern void liftLock(void);
extern void liftUnlock(void);

#ifdef __cplusplus
}
#endif

#endif
