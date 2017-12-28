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
static void liftPIDUpdate(int16_t *cmd);

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
/** @brief      Assign motor and potentiometer to the lift system.             */
/** @param[in]  motor0 The lift first motor pair                               */
/** @param[in]  motor1 The lift second motor pair                              */
/** @param[in]  potentiometer The lift potentiometer                           */
/** @param[in]  reversed Is the lift potentiometer reversed?                   */
/** @param[in]  gearRatio Gear ratio between motor and potentiometer           */
/** @param[in]  downValue The lift potentiometer down value                    */
/** @param[in]  bumpValue The lift potentiometer bump value                    */
/** @param[in]  upValue The lift potentiometer up value                        */
/*-----------------------------------------------------------------------------*/
void
liftSetup(tVexMotor motor0, tVexMotor motor1, tVexAnalogPin potentiometer, bool reversed, float gearRatio, int16_t floorValue,
          int16_t carryValue, int16_t ceilingValue)
{
    lift.motor0 = motor0;
    lift.motor1 = motor1;
    lift.potentiometer = potentiometer;
    lift.reversed = reversed;
    lift.gearRatio = gearRatio;
    lift.floorValue = floorValue;
    lift.carryValue = carryValue;
    lift.ceilingValue = ceilingValue;
    lift.command = liftCommandFree;
    lift.locked = true;
    lift.lock = NULL;
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize the lift system.                                    */
/*-----------------------------------------------------------------------------*/
void
liftInit(void)
{
    lift.lock = PidControllerInit(0.004, 0.0001, 0.01, kVexSensorUndefined, 0);
    lift.lock->enabled = 0;
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
    int16_t liftCmd = 0;
    bool immediate = false;

    // Unused
    (void)arg;

    // Register the task
    vexTaskRegister("lift");

    while (!chThdShouldTerminate()) {
        if (lift.locked) {
            liftCmd = liftSpeed(limitSpeed(vexControllerGet(Ch2Xmtr2), 20));

            if (liftCmd == 0) {
                immediate = false;
                if (vexControllerGet(Btn7D) || vexControllerGet(Btn7DXmtr2)) {
                    lift.command = liftCommandFloor;
                    lift.lock->enabled = 1;
                    lift.lock->target_value = lift.floorValue;
                } else if (vexControllerGet(Btn7L) || vexControllerGet(Btn7LXmtr2)) {
                    lift.command = liftCommandCarry;
                    lift.lock->enabled = 1;
                    lift.lock->target_value = lift.carryValue;
                } else if (vexControllerGet(Btn7U) || vexControllerGet(Btn7UXmtr2)) {
                    lift.command = liftCommandCeiling;
                    lift.lock->enabled = 1;
                    lift.lock->target_value = lift.ceilingValue;
                }
                liftPIDUpdate(&liftCmd);
            } else {
                lift.command = liftCommandFree;
                immediate = true;
                // disable PID if joystick driving
                lift.lock->enabled = 0;
                PidControllerUpdate(lift.lock); // zero out PID
            }

            liftMove(liftCmd, immediate);
        }

        // Don't hog cpu
        vexSleep(25);
    }

    return ((msg_t)0);
}

static void
liftPIDUpdate(int16_t *cmdp)
{
    int16_t cmd;
    // enable PID if not driving and already disabled
    if (lift.lock->enabled == 0) {
        lift.lock->enabled = 1;
        lift.lock->target_value = vexAdcGet(lift.potentiometer);
    }
    // prevent PID from trying to lock outside bounds
    if (lift.reversed) {
        if (lift.lock->target_value > lift.floorValue)
            lift.lock->target_value = lift.floorValue;
        else if (lift.lock->target_value < lift.ceilingValue)
            lift.lock->target_value = lift.ceilingValue;
    } else {
        if (lift.lock->target_value < lift.floorValue)
            lift.lock->target_value = lift.floorValue;
        else if (lift.lock->target_value > lift.ceilingValue)
            lift.lock->target_value = lift.ceilingValue;
    }
    // update PID
    lift.lock->sensor_value = vexAdcGet(lift.potentiometer);
    lift.lock->error =
        (lift.reversed) ? (lift.lock->sensor_value - lift.lock->target_value) : (lift.lock->target_value - lift.lock->sensor_value);
    cmd = PidControllerUpdate(lift.lock);
    cmd = liftSpeed(cmd);
    *cmdp = cmd;
    return;
}

void
liftMove(int16_t cmd, bool immediate)
{
    SetMotor(lift.motor0, cmd, immediate);
    SetMotor(lift.motor1, cmd, immediate);
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

void
liftLockFloor(void)
{
    liftLock();
    lift.command = liftCommandFloor;
    lift.lock->enabled = 1;
    lift.lock->target_value = lift.floorValue;
}

void
liftLockCarry(void)
{
    liftLock();
    lift.command = liftCommandCarry;
    lift.lock->enabled = 1;
    lift.lock->target_value = lift.carryValue;
}

void
liftLockCeiling(void)
{
    liftLock();
    lift.command = liftCommandCeiling;
    lift.lock->enabled = 1;
    lift.lock->target_value = lift.ceilingValue;
}

void
liftLockCurrent(void)
{
    liftLock();
    lift.command = liftCommandFree;
    lift.lock->enabled = 1;
    lift.lock->target_value = vexAdcGet(lift.potentiometer);
}
