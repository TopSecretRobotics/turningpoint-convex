// -*- mo----de: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*-----------------------------------------------------------------------------*/
/** @file    system.c                                                          */
/** @brief   The system manager for the robot                                  */
/*-----------------------------------------------------------------------------*/

#include "system.h"

#include "lcd.h"

#include "arm.h"
#include "drive.h"
#include "intake.h"
#include "lift.h"
#include "setter.h"

// storage for system manager
static const system_t systems[] = {
    {true, lcdInit, lcdStart, NULL, NULL},
    {true, armInit, armStart, armLock, armUnlock},
    {true, driveInit, driveStart, driveLock, driveUnlock},
    {true, intakeInit, intakeStart, intakeLock, intakeUnlock},
    {true, liftInit, liftStart, liftLockCurrent, liftUnlock},
    {true, setterInit, setterStart, setterLock, setterUnlock},
    {false, NULL, NULL, NULL, NULL},
};

static bool systemIsEmpty(const system_t *system);

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize all enabled systems                                 */
/*-----------------------------------------------------------------------------*/
void
systemInitAll(void)
{
    const system_t *system = systems;
    while (!systemIsEmpty(system)) {
        if (system->enabled == true && system->init != NULL) {
            system->init();
        }
        system++;
    }
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start all enabled systems                                      */
/*-----------------------------------------------------------------------------*/
void
systemStartAll(void)
{
    const system_t *system = systems;
    while (!systemIsEmpty(system)) {
        if (system->enabled == true && system->start != NULL) {
            system->start();
        }
        system++;
    }
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Lock all enabled systems                                       */
/*-----------------------------------------------------------------------------*/
void
systemLockAll(void)
{
    const system_t *system = systems;
    while (!systemIsEmpty(system)) {
        if (system->enabled == true && system->lock != NULL) {
            system->lock();
        }
        system++;
    }
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Unlock all enabled systems                                     */
/*-----------------------------------------------------------------------------*/
void
systemUnlockAll(void)
{
    const system_t *system = systems;
    while (!systemIsEmpty(system)) {
        if (system->enabled == true && system->unlock != NULL) {
            system->unlock();
        }
        system++;
    }
    return;
}

// Inline functions

inline bool
systemIsEmpty(const system_t *system)
{
    if (system == NULL || (system->init == NULL && system->start == NULL && system->lock == NULL && system->unlock == NULL)) {
        return true;
    } else {
        return false;
    }
}
