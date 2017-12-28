// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*
 * setter.h
 */

#ifndef SETTER_H_

#define SETTER_H_

#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header
#include "vex.h" // vex library header

#include "smartmotor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct setter_s {
    tVexMotor motor;
    bool locked;
} setter_t;

extern setter_t *setterGetPtr(void);
extern void setterSetup(tVexMotor motor);
extern void setterInit(void);
extern void setterStart(void);
extern void setterMove(int16_t cmd, bool immediate);
extern void setterLock(void);
extern void setterUnlock(void);

#ifdef __cplusplus
}
#endif

#endif
