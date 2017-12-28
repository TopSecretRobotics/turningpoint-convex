// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*
 * intake.h
 */

#ifndef INTAKE_H_

#define INTAKE_H_

#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header
#include "vex.h" // vex library header

#include "smartmotor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct intake_s {
    tVexMotor motor;
    bool locked;
} intake_t;

extern intake_t *intakeGetPtr(void);
extern void intakeSetup(tVexMotor motor);
extern void intakeInit(void);
extern void intakeStart(void);
extern void intakeMove(int16_t cmd, bool immediate);
extern void intakeLock(void);
extern void intakeUnlock(void);

#ifdef __cplusplus
}
#endif

#endif
