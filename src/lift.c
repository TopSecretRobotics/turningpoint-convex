// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*-----------------------------------------------------------------------------*/
/** @file    lift.c                                                            */
/** @brief   The lift system for the robot                                     */
/*-----------------------------------------------------------------------------*/

#include "lift.h"

#include <math.h>
#include <stdlib.h>

// storage for lift
static lift_t lift;

// working area for lift task
static WORKING_AREA(waLift, 512);

// private functions
static msg_t liftThread(void *arg);

// lift speed adjustment
#define USE_LIFT_SPEED_TABLE 1
#ifdef USE_LIFT_SPEED_TABLE

const unsigned int liftSpeedTable[128] = {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  21, 21, 21, 22, 22,  22,  23, 24, 24, 25, 25,
                                          25, 25, 26, 27, 27, 28, 28, 28, 28, 29, 30, 30, 30, 31, 31, 32,  32,  32, 33, 33, 34, 34,
                                          35, 35, 35, 36, 36, 37, 37, 37, 37, 38, 38, 39, 39, 39, 40, 40,  41,  41, 42, 42, 43, 44,
                                          44, 45, 45, 46, 46, 47, 47, 48, 48, 49, 50, 50, 51, 52, 52, 53,  54,  55, 56, 57, 57, 58,
                                          59, 60, 61, 62, 63, 64, 65, 66, 67, 67, 68, 70, 71, 72, 72, 73,  74,  76, 77, 78, 79, 79,
                                          80, 81, 83, 84, 84, 86, 86, 87, 87, 88, 88, 89, 89, 90, 90, 127, 127, 127};

static inline int
liftSpeed(int speed)
{
    if (speed > 127)
        speed = 127;
    else if (speed < -127)
        speed = -127;
    return (((speed > 0) - (speed < 0)) * liftSpeedTable[abs(speed)]);
}

#else

static inline int
liftSpeed(int speed)
{
    if (speed > 127)
        speed = 127;
    else if (speed < -127)
        speed = -127;
    return (speed);
}

#endif

/*-----------------------------------------------------------------------------*/
/** @brief      Get pointer to lift structure - not used locally               */
/** @return     A lift_t pointer                                               */
/*-----------------------------------------------------------------------------*/
lift_t *
liftGetPtr(void)
{
    return (&lift);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Assign motor and limit sensor to the lift system.              */
/** @param[in]  motor The lift motor                                           */
/** @param[in]  limit The lift limit sensor                                    */
/*-----------------------------------------------------------------------------*/
void
liftSetup(tVexMotor motor, tVexDigitalPin limit)
{
    lift.motor = motor;
    lift.limit = limit;
    lift.locked = true;
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize the lift system.                                    */
/*-----------------------------------------------------------------------------*/
void
liftInit(void)
{
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start the lift system thread                                   */
/*-----------------------------------------------------------------------------*/
void
liftStart(void)
{
    chThdCreateStatic(waLift, sizeof(waLift), NORMALPRIO - 1, liftThread, NULL);
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
/** @brief      The lift system thread                                         */
/** @param[in]  arg Unused                                                     */
/** @return     (msg_t) 0                                                      */
/*-----------------------------------------------------------------------------*/
static msg_t
liftThread(void *arg)
{
    bool buttonHold = false;
    bool buttonOff = false;
    int16_t liftCmd = 0;
    bool immediate = false;

    // Unused
    (void)arg;

    // Register the task
    vexTaskRegister("lift");

    while (!chThdShouldTerminate()) {
        if (lift.locked) {
            buttonHold = vexControllerGet(Btn7DXmtr2);
            buttonOff = vexControllerGet(Btn7UXmtr2);
            if (buttonOff == true) {
                liftCmd = 0;
                immediate = true;
            } else {
                if (buttonHold == true) {
                    liftCmd = 127;
                    immediate = true;
                } else if (vexDigitalPinGet(lift.limit) == kVexDigitalHigh) {
                    liftCmd = 0;
                    immediate = true;
                } else {
                    liftCmd = 127;
                    immediate = false;
                }
            }
            liftCmd = liftSpeed(liftCmd);
            liftMove(liftCmd, immediate);
        }

        // Don't hog cpu
        vexSleep(25);
    }

    return ((msg_t)0);
}

void
liftMove(int16_t cmd, bool immediate)
{
    SetMotor(lift.motor, cmd, immediate);
}

void
liftLock(void)
{
    lift.locked = true;
}

void
liftUnlock(void)
{
    lift.locked = false;
}
