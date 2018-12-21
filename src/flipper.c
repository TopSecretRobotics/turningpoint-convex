

#include "flipper.h"

#include <math.h>
#include <stdlib.h>

// storage for flipper
static flipper_t flipper;

// working area for flipper task
static WORKING_AREA(waflipper, 512);

// private functions
static msg_t flipperThread(void *arg);

// flipper speed adjustment
#define USE_flipper_SPEED_TABLE 1
#ifdef USE_flipper_SPEED_TABLE

const unsigned int flipperSpeedTable[128] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  21, 21, 21, 22, 22, 22, 23, 24, 24, 25, 25,  25,  25, 26, 27,
    27, 28, 28, 28, 28, 29, 30, 30, 30, 31, 31, 32, 32, 32, 33, 33, 34, 34, 35, 35, 35, 36,  36,  37, 37, 37,
    37, 38, 38, 39, 39, 39, 40, 40, 41, 41, 42, 42, 43, 44, 44, 45, 45, 46, 46, 47, 47, 48,  48,  49, 50, 50,
    51, 52, 52, 53, 54, 55, 56, 57, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 67, 68, 70,  71,  72, 72, 73,
    74, 76, 77, 78, 79, 79, 80, 81, 83, 84, 84, 86, 86, 87, 87, 88, 88, 89, 89, 90, 90, 127, 127, 127};

static inline int
flipperSpeed(int speed)
{
    if (speed > 127)
        speed = 127;
    else if (speed < -127)
        speed = -127;
    return (((speed > 0) - (speed < 0)) * flipperSpeedTable[abs(speed)]);
}

#else

static inline int
flipperSpeed(int speed)
{
    if (speed > 127)
        speed = 127;
    else if (speed < -127)
        speed = -127;
    return (speed);
}

#endif

/*-----------------------------------------------------------------------------*/
/** @brief      Get pointer to flipper structure - not used locally             */
/** @return     A flipper_t pointer                                             */
/*-----------------------------------------------------------------------------*/
flipper_t *
flipperGetPtr(void)
{
    return (&flipper);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Assign motor to the flipper system.                             */
/** @param[in]  motor The flipper motor                                         */
/*-----------------------------------------------------------------------------*/
void
flipperSetup(tVexMotor motor)
{
    flipper.motor = motor;
    flipper.locked = true;
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize the flipper system.                                  */
/*-----------------------------------------------------------------------------*/
void
flipperInit(void)
{
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start the flipper system thread                                 */
/*-----------------------------------------------------------------------------*/
void
flipperStart(void)
{
    chThdCreateStatic(waflipper, sizeof(waflipper), NORMALPRIO - 1, flipperThread, NULL);
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
/** @brief      The flipper system thread                                       */
/** @param[in]  arg Unused                                                     */
/** @return     (msg_t) 0                                                      */
/*-----------------------------------------------------------------------------*/
static msg_t
flipperThread(void *arg)
{
    bool buttonIn = false;
    bool buttonOut = false;
    int16_t flipperCmd = 0;
    bool immediate = false;

    // Unused
    (void)arg;

    // Register the task
    vexTaskRegister("flipper");

    while (!chThdShouldTerminate()) {
        if (flipper.locked) {
            buttonIn = (bool)vexControllerGet(Btn8D);
            buttonOut = (bool)vexControllerGet(Btn8L);
            if (vexControllerGet(Btn5UXmtr2)) {
                buttonIn = (bool)vexControllerGet(Btn8DXmtr2);
                buttonOut = (bool)vexControllerGet(Btn8LXmtr2);
            }
            if (buttonIn == true) {
                flipperCmd = 127;
            } else if (buttonOut == true) {
                flipperCmd = -127;
            } else {
                flipperCmd = 0;
            }
            flipperCmd = flipperSpeed(flipperCmd);
            flipperMove(flipperCmd, immediate);
        }

        // Don't hog cpu
        vexSleep(25);
    }

    return ((msg_t)0);
}

void
flipperMove(int16_t cmd, bool immediate)
{
    SetMotor(flipper.motor, cmd, immediate);
}

void
flipperLock(void)
{
    flipper.locked = true;
}

void
flipperUnlock(void)
{
    flipper.locked = false;
}
