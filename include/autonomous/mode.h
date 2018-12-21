// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*
 * autonomous/mode.h
 */

#ifndef AUTONOMOUS_MODE_H_

#define AUTONOMOUS_MODE_H_

#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header
#include "vex.h" // vex library header

#include "pidlib.h"
#include "smartmotor.h"

#include "arm.h"
#include "drive.h"
#include "intake.h"
#include "flipper.h"
#include "lift.h"
#include "setter.h"

#include "system.h"

#include <math.h>
#include <stdlib.h>

/* Types and variables */

#define ROBOT_ARM 0x01
#define ROBOT_DRIVE 0x02
#define ROBOT_INTAKE 0x04
#define ROBOT_FLIPPER 0x06
#define ROBOT_LIFT 0x08
#define ROBOT_SETTER 0x10
#define ROBOT_ALL (ROBOT_ARM | ROBOT_DRIVE | ROBOT_INTAKE | ROBOT_FLIPPER | ROBOT_LIFT | ROBOT_SETTER)

#include "autonomous/timer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Arm function declarations */
static void armRaise(int speed);
static void armLower(int speed);
/* Drive function declarations */
static void driveForward(int speed);
static void driveBackward(int speed);
static void driveRight(int speed);
static void driveLeft(int speed);
/* Intake function declarations */
static void intakeGrab(int speed);
static void intakeRelease(int speed);
/* Flipper function declarations */
static void flipperFlip(int speed);
/* Lift function declarations */
static void liftRaise(int speed);
static void liftLower(int speed);
/* Setter function declarations */
static void setterGrab(int speed);
static void setterOpen(int speed);
/* Stop Movement function declarations */
static void stopMovementOf(int robot, unsigned long timeout);
static void stopMovementAll(unsigned long timeout);
static void stopMovementDrive(unsigned long timeout);

/* Arm function definitions */

inline void
armRaise(int speed)
{
    armMove(speed, true);
}

inline void
armLower(int speed)
{
    armMove(-speed, true);
}

/* Drive function definitions */

inline void
driveForward(int speed)
{
    driveMove(0, speed, true);
}

inline void
driveBackward(int speed)
{
    driveMove(0, -speed, true);
}

inline void
driveRight(int speed)
{
    driveMove(speed, 0, true);
}

inline void
driveLeft(int speed)
{
    driveMove(-speed, 0, true);
}

/* Intake function definitions */

inline void
intakeGrab(int speed)
{
    intakeMove(speed, true);
}

inline void
intakeRelease(int speed)
{
    intakeMove(-speed, true);
}

inline void
flipperFlip(int speed)
{
    flipperMove(speed, true);
}

/* Lift function definitions */

inline void
liftRaise(int speed)
{
    liftMove(speed, true);
}

inline void
liftLower(int speed)
{
    liftMove(-speed, true);
}

/* Setter function definitions */

inline void
setterGrab(int speed)
{
    setterMove(speed, true);
}

inline void
setterOpen(int speed)
{
    setterMove(-speed, true);
}

/* Stop Movement function definitions */

/**
 * Example of stopping all movement of drive system for 5 seconds:
 *
 *   stopMovementOf(ROBOT_DRIVE, 5000);
 *
 * Example of stopping all movement of arm, drive, and intake systems for 5 seconds:
 *
 *   stopMovementOf((ROBOT_ARM | ROBOT_DRIVE | ROBOT_INTAKE), 5000);
 *
 * Example of stopping all movement of all systems for 5 seconds:
 *
 *   stopMovementOf(ROBOT_ALL, 5000);
 */
inline void
stopMovementOf(int robot, unsigned long timeout)
{
    timerRun(timeout, {
        if (robot & ROBOT_ARM) {
            armMove(0, true);
        }
        if (robot & ROBOT_DRIVE) {
            driveMove(0, 0, true);
        }
        if (robot & ROBOT_INTAKE) {
            intakeMove(0, true);
        }
        if (robot & ROBOT_FLIPPER) {
            flipperMove(0, true);
        }
        if (robot & ROBOT_LIFT) {
            liftMove(0, true);
        }
        if (robot & ROBOT_SETTER) {
            setterMove(0, true);
        }
    });
    return;
}

inline void
stopMovementAll(unsigned long timeout)
{
    (void)stopMovementOf(ROBOT_ALL, timeout);
    return;
}

inline void
stopMovementDrive(unsigned long timeout)
{
    (void)stopMovementOf(ROBOT_DRIVE, timeout);
    return;
}

#ifdef __cplusplus
}
#endif

#endif
