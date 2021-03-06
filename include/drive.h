// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*
 * drive.h
 */

#ifndef DRIVE_H_

#define DRIVE_H_

#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header
#include "vex.h" // vex library header

#include "smartmotor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct drive_s {
    tVexMotor northeast;
    tVexMotor northwest;
    tVexMotor southeast;
    tVexMotor southwest;
    bool locked;
} drive_t;

extern drive_t *driveGetPtr(void);
extern void driveSetup(tVexMotor northeast, tVexMotor northwest, tVexMotor southeast, tVexMotor southwest);
extern void driveInit(void);
extern void driveStart(void);
extern void driveMove(int16_t x, int16_t y, bool immediate);
extern void driveLock(void);
extern void driveUnlock(void);

#ifdef __cplusplus
}
#endif

#endif
