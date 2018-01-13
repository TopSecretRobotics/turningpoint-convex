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

typedef enum { armCommandFree = -1, armCommandFloor = 0, armCommandPickup, armCommandCarry, armCommandCeiling } armCommand_t;

typedef struct arm_s {
    tVexMotor motor;
    tVexAnalogPin potentiometer;
    bool reversed;
    float gearRatio;
    int16_t floorValue;
    int16_t pickupValue;
    int16_t carryValue;
    int16_t ceilingValue;
    armCommand_t command;
    bool locked;
    pidController *lock;
} arm_t;

extern arm_t *armGetPtr(void);
extern void armSetup(tVexMotor motor, tVexAnalogPin potentiometer, bool reversed, float gearRatio, int16_t floorValue,
                     int16_t pickupValue, int16_t carryValue, int16_t ceilingValue);
extern void armInit(void);
extern void armStart(void);
extern void armMove(int16_t cmd, bool immediate);
extern void armLock(void);
extern void armUnlock(void);
extern void armLockFloor(void);
extern void armLockPickup(void);
extern void armLockCarry(void);
extern void armLockCeiling(void);
extern void armLockCurrent(void);

#ifdef __cplusplus
}
#endif

#endif
