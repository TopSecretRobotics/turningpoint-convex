// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*-----------------------------------------------------------------------------*/
/** @file    intake.c                                                          */
/** @brief   The intake system for the robot                                   */
/*-----------------------------------------------------------------------------*/

#include "intake.h"

#include <math.h>
#include <stdlib.h>

// storage for intake
static intake_t intake;

// working area for intake task
static WORKING_AREA(waIntake, 512);

// private functions
static msg_t intakeThread(void *arg);

// intake speed adjustment
#define USE_INTAKE_SPEED_TABLE 1
#ifdef USE_INTAKE_SPEED_TABLE

const unsigned int intakeSpeedTable[128] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  21, 21, 21, 22, 22, 22, 23, 24, 24, 25, 25,  25,  25, 26, 27,
    27, 28, 28, 28, 28, 29, 30, 30, 30, 31, 31, 32, 32, 32, 33, 33, 34, 34, 35, 35, 35, 36,  36,  37, 37, 37,
    37, 38, 38, 39, 39, 39, 40, 40, 41, 41, 42, 42, 43, 44, 44, 45, 45, 46, 46, 47, 47, 48,  48,  49, 50, 50,
    51, 52, 52, 53, 54, 55, 56, 57, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 67, 68, 70,  71,  72, 72, 73,
    74, 76, 77, 78, 79, 79, 80, 81, 83, 84, 84, 86, 86, 87, 87, 88, 88, 89, 89, 90, 90, 127, 127, 127};

static inline int
intakeSpeed(int speed)
{
    if (speed > 127)
        speed = 127;
    else if (speed < -127)
        speed = -127;
    return (((speed > 0) - (speed < 0)) * intakeSpeedTable[abs(speed)]);
}

#else

static inline int
intakeSpeed(int speed)
{
    if (speed > 127)
        speed = 127;
    else if (speed < -127)
        speed = -127;
    return (speed);
}

#endif

/*-----------------------------------------------------------------------------*/
/** @brief      Get pointer to intake structure - not used locally             */
/** @return     A intake_t pointer                                             */
/*-----------------------------------------------------------------------------*/
intake_t *
intakeGetPtr(void)
{
    return (&intake);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Assign motor to the intake system.                             */
/** @param[in]  motor The intake motor                                         */
/*-----------------------------------------------------------------------------*/
void
intakeSetup(tVexMotor motor)
{
    intake.motor = motor;
    intake.locked = true;
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize the intake system.                                  */
/*-----------------------------------------------------------------------------*/
void
intakeInit(void)
{
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start the intake system thread                                 */
/*-----------------------------------------------------------------------------*/
void
intakeStart(void)
{
    chThdCreateStatic(waIntake, sizeof(waIntake), NORMALPRIO - 1, intakeThread, NULL);
    return;
}

static inline int
limitSpeed(int speed, int limit)
{
    if (abs(speed) <= limit) {
        return 0;
    }
    return speed;
}

/*-----------------------------------------------------------------------------*/
/** @brief      The intake system thread                                       */
/** @param[in]  arg Unused                                                     */
/** @return     (msg_t) 0                                                      */
/*-----------------------------------------------------------------------------*/
static msg_t
intakeThread(void *arg)
{
    int16_t intakeCmd = 0;
    bool immediate = false;

    // Unused
    (void)arg;

    // Register the task
    vexTaskRegister("intake");

    while (!chThdShouldTerminate()) {
        if (intake.locked) {
            intakeCmd = intakeSpeed(limitSpeed(vexControllerGet(Ch2Xmtr2), 20));
            intakeMove(intakeCmd, immediate);
        }

        // Don't hog cpu
        vexSleep(25);
    }

    return ((msg_t)0);
}

void
intakeMove(int16_t cmd, bool immediate)
{
    SetMotor(intake.motor, cmd, immediate);
}

void
intakeLock(void)
{
    intake.locked = true;
}

void
intakeUnlock(void)
{
    intake.locked = false;
}
