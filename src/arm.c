// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*-----------------------------------------------------------------------------*/
/** @file    arm.c                                                             */
/** @brief   The arm system for the robot                                      */
/*-----------------------------------------------------------------------------*/

#include "arm.h"

#include <math.h>
#include <stdlib.h>

// storage for arm
static arm_t arm;

// working area for arm task
static WORKING_AREA(waArm, 512);

// private functions
static msg_t armThread(void *arg);
static void armPIDUpdate(int16_t *cmd);

// arm speed adjustment
#define USE_ARM_SPEED_TABLE 1
#ifdef USE_ARM_SPEED_TABLE

const unsigned int armSpeedTable[128] = {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  21, 21, 21, 22, 22,  22,  23, 24, 24, 25, 25,
                                         25, 25, 26, 27, 27, 28, 28, 28, 28, 29, 30, 30, 30, 31, 31, 32,  32,  32, 33, 33, 34, 34,
                                         35, 35, 35, 36, 36, 37, 37, 37, 37, 38, 38, 39, 39, 39, 40, 40,  41,  41, 42, 42, 43, 44,
                                         44, 45, 45, 46, 46, 47, 47, 48, 48, 49, 50, 50, 51, 52, 52, 53,  54,  55, 56, 57, 57, 58,
                                         59, 60, 61, 62, 63, 64, 65, 66, 67, 67, 68, 70, 71, 72, 72, 73,  74,  76, 77, 78, 79, 79,
                                         80, 81, 83, 84, 84, 86, 86, 87, 87, 88, 88, 89, 89, 90, 90, 127, 127, 127};

static inline int
armSpeed(int speed)
{
    if (speed > 127)
        speed = 127;
    else if (speed < -127)
        speed = -127;
    return (((speed > 0) - (speed < 0)) * armSpeedTable[abs(speed)]);
}

#else

static inline int
armSpeed(int speed)
{
    if (speed > 127)
        speed = 127;
    else if (speed < -127)
        speed = -127;
    return (speed);
}

#endif

/*-----------------------------------------------------------------------------*/
/** @brief      Get pointer to arm structure - not used locally                */
/** @return     A arm_t pointer                                                */
/*-----------------------------------------------------------------------------*/
arm_t *
armGetPtr(void)
{
    return (&arm);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Assign motor and potentiometer to the arm system.              */
/** @param[in]  motor The arm motor (possibly a y-cabled pair)                 */
/** @param[in]  potentiometer The arm potentiometer                            */
/** @param[in]  reversed Is the arm potentiometer reversed?                    */
/** @param[in]  gearRatio Gear ratio between motor and potentiometer           */
/** @param[in]  floorValue The arm potentiometer floor value                   */
/** @param[in]  pickupValue The arm potentiometer pickup value                 */
/** @param[in]  carryValue The arm potentiometer carry value                   */
/** @param[in]  ceilingValue The arm potentiometer ceiling value               */
/*-----------------------------------------------------------------------------*/
void
armSetup(tVexMotor motor, tVexAnalogPin potentiometer, bool reversed, float gearRatio, int16_t floorValue, int16_t pickupValue,
         int16_t carryValue, int16_t ceilingValue)
{
    arm.motor = motor;
    arm.potentiometer = potentiometer;
    arm.reversed = reversed;
    arm.gearRatio = gearRatio;
    arm.floorValue = floorValue;
    arm.pickupValue = pickupValue;
    arm.carryValue = carryValue;
    arm.ceilingValue = ceilingValue;
    arm.command = armCommandFree;
    arm.locked = true;
    arm.lock = NULL;
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize the arm system.                                     */
/*-----------------------------------------------------------------------------*/
void
armInit(void)
{
    arm.lock = PidControllerInit(0.004, 0.0001, 0.01, kVexSensorUndefined, 0);
    arm.lock->enabled = 0;
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start the arm system thread                                    */
/*-----------------------------------------------------------------------------*/
void
armStart(void)
{
    chThdCreateStatic(waArm, sizeof(waArm), NORMALPRIO - 1, armThread, NULL);
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
/** @brief      The arm system thread                                          */
/** @param[in]  arg Unused                                                     */
/** @return     (msg_t) 0                                                      */
/*-----------------------------------------------------------------------------*/
static msg_t
armThread(void *arg)
{
    int16_t armCmd = 0;
    bool immediate = false;

    // Unused
    (void)arg;

    // Register the task
    vexTaskRegister("arm");

    while (!chThdShouldTerminate()) {
        if (arm.locked) {
            armCmd = vexControllerGet(Ch2Xmtr2);
            if (vexControllerGet(Btn5U)) {
                armCmd = vexControllerGet(Ch2);
            }
            armCmd = armSpeed(limitSpeed(armCmd, 20));

            if (armCmd == 0) {
                immediate = false;
                if (vexControllerGet(Btn8D) || vexControllerGet(Btn8DXmtr2)) {
                    arm.command = armCommandFloor;
                    arm.lock->enabled = 1;
                    arm.lock->target_value = arm.floorValue;
                } else if (vexControllerGet(Btn8L) || vexControllerGet(Btn8LXmtr2)) {
                    arm.command = armCommandPickup;
                    arm.lock->enabled = 1;
                    arm.lock->target_value = arm.pickupValue;
                } else if (vexControllerGet(Btn8R) || vexControllerGet(Btn8RXmtr2)) {
                    arm.command = armCommandCarry;
                    arm.lock->enabled = 1;
                    arm.lock->target_value = arm.carryValue;
                } else if (vexControllerGet(Btn8U) || vexControllerGet(Btn8UXmtr2)) {
                    arm.command = armCommandCeiling;
                    arm.lock->enabled = 1;
                    arm.lock->target_value = arm.ceilingValue;
                }
                armPIDUpdate(&armCmd);
            } else {
                arm.command = armCommandFree;
                immediate = true;
                // disable PID if joystick driving
                arm.lock->enabled = 0;
                PidControllerUpdate(arm.lock); // zero out PID
            }

            armMove(armCmd, immediate);
        }

        // Don't hog cpu
        vexSleep(25);
    }

    return ((msg_t)0);
}

static void
armPIDUpdate(int16_t *cmdp)
{
    int16_t cmd;
    // enable PID if not driving and already disabled
    if (arm.lock->enabled == 0) {
        arm.lock->enabled = 1;
        arm.lock->target_value = vexAdcGet(arm.potentiometer);
    }
    // prevent PID from trying to lock outside bounds
    if (arm.reversed) {
        if (arm.lock->target_value > arm.floorValue)
            arm.lock->target_value = arm.floorValue;
        else if (arm.lock->target_value < arm.ceilingValue)
            arm.lock->target_value = arm.ceilingValue;
    } else {
        if (arm.lock->target_value < arm.floorValue)
            arm.lock->target_value = arm.floorValue;
        else if (arm.lock->target_value > arm.ceilingValue)
            arm.lock->target_value = arm.ceilingValue;
    }
    // update PID
    arm.lock->sensor_value = vexAdcGet(arm.potentiometer);
    arm.lock->error =
        (arm.reversed) ? (arm.lock->sensor_value - arm.lock->target_value) : (arm.lock->target_value - arm.lock->sensor_value);
    cmd = PidControllerUpdate(arm.lock);
    cmd = armSpeed(cmd);
    *cmdp = cmd;
    return;
}

void
armMove(int16_t cmd, bool immediate)
{
    SetMotor(arm.motor, cmd, immediate);
}

void
armLock(void)
{
    arm.locked = true;
}

void
armUnlock(void)
{
    arm.locked = false;
}

void
armLockFloor(void)
{
    armLock();
    arm.command = armCommandFloor;
    arm.lock->enabled = 1;
    arm.lock->target_value = arm.floorValue;
}

void
armLockPickup(void)
{
    armLock();
    arm.command = armCommandPickup;
    arm.lock->enabled = 1;
    arm.lock->target_value = arm.pickupValue;
}

void
armLockCarry(void)
{
    armLock();
    arm.command = armCommandCarry;
    arm.lock->enabled = 1;
    arm.lock->target_value = arm.carryValue;
}

void
armLockCeiling(void)
{
    armLock();
    arm.command = armCommandCeiling;
    arm.lock->enabled = 1;
    arm.lock->target_value = arm.ceilingValue;
}

void
armLockCurrent(void)
{
    armLock();
    arm.command = armCommandFree;
    arm.lock->enabled = 1;
    arm.lock->target_value = vexAdcGet(arm.potentiometer);
}
