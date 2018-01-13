// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*-----------------------------------------------------------------------------*/
/** @file    setter.c                                                          */
/** @brief   The setter system for the robot                                   */
/*-----------------------------------------------------------------------------*/

#include "setter.h"

#include <math.h>
#include <stdlib.h>

// storage for setter
static setter_t setter;

// working area for setter task
static WORKING_AREA(waSetter, 512);

// private functions
static msg_t setterThread(void *arg);

// setter speed adjustment
#define USE_SETTER_SPEED_TABLE 1
#ifdef USE_SETTER_SPEED_TABLE

const unsigned int setterSpeedTable[128] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  21, 21, 21, 22, 22, 22, 23, 24, 24, 25, 25,  25,  25, 26, 27,
    27, 28, 28, 28, 28, 29, 30, 30, 30, 31, 31, 32, 32, 32, 33, 33, 34, 34, 35, 35, 35, 36,  36,  37, 37, 37,
    37, 38, 38, 39, 39, 39, 40, 40, 41, 41, 42, 42, 43, 44, 44, 45, 45, 46, 46, 47, 47, 48,  48,  49, 50, 50,
    51, 52, 52, 53, 54, 55, 56, 57, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 67, 68, 70,  71,  72, 72, 73,
    74, 76, 77, 78, 79, 79, 80, 81, 83, 84, 84, 86, 86, 87, 87, 88, 88, 89, 89, 90, 90, 127, 127, 127};

static inline int
setterSpeed(int speed)
{
    if (speed > 127)
        speed = 127;
    else if (speed < -127)
        speed = -127;
    return (((speed > 0) - (speed < 0)) * setterSpeedTable[abs(speed)]);
}

#else

static inline int
setterSpeed(int speed)
{
    if (speed > 127)
        speed = 127;
    else if (speed < -127)
        speed = -127;
    return (speed);
}

#endif

/*-----------------------------------------------------------------------------*/
/** @brief      Get pointer to setter structure - not used locally             */
/** @return     A setter_t pointer                                             */
/*-----------------------------------------------------------------------------*/
setter_t *
setterGetPtr(void)
{
    return (&setter);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Assign motor to the setter system.                             */
/** @param[in]  motor The setter motor                                         */
/*-----------------------------------------------------------------------------*/
void
setterSetup(tVexMotor motor)
{
    setter.motor = motor;
    setter.locked = true;
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize the setter system.                                  */
/*-----------------------------------------------------------------------------*/
void
setterInit(void)
{
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start the setter system thread                                 */
/*-----------------------------------------------------------------------------*/
void
setterStart(void)
{
    chThdCreateStatic(waSetter, sizeof(waSetter), NORMALPRIO - 1, setterThread, NULL);
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
/** @brief      The setter system thread                                       */
/** @param[in]  arg Unused                                                     */
/** @return     (msg_t) 0                                                      */
/*-----------------------------------------------------------------------------*/
static msg_t
setterThread(void *arg)
{
    int16_t setterCmd = 0;
    bool immediate = true;

    // Unused
    (void)arg;

    // Register the task
    vexTaskRegister("setter");

    while (!chThdShouldTerminate()) {
        if (setter.locked) {
            setterCmd = 0;
            if (vexControllerGet(Btn5U)) {
                setterCmd = vexControllerGet(Ch2);
            }
            setterCmd = setterSpeed(limitSpeed(setterCmd, 20));
            setterMove(setterCmd, immediate);
        }

        // Don't hog cpu
        vexSleep(25);
    }

    return ((msg_t)0);
}

void
setterMove(int16_t cmd, bool immediate)
{
    SetMotor(setter.motor, cmd, immediate);
}

void
setterLock(void)
{
    setter.locked = true;
}

void
setterUnlock(void)
{
    setter.locked = false;
}
