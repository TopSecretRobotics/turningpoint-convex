// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*-----------------------------------------------------------------------------*/
/** @file    system.c                                                          */
/** @brief   The system manager for the robot                                  */
/*-----------------------------------------------------------------------------*/

#include "system.h"

#include "arm.h"
#include "drive.h"
#include "intake.h"
#include "lift.h"
#include "setter.h"

// storage for system manager
static system_t systems[] = {
    {true, armInit, armStart, armLock, armUnlock},
    {true, driveInit, driveStart, driveLock, driveUnlock},
    {true, intakeInit, intakeStart, intakeLock, intakeUnlock},
    {true, liftInit, liftStart, liftLock, liftUnlock},
    {true, setterInit, setterStart, setterLock, setterUnlock},
    {false, NULL, NULL, NULL, NULL},
};

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize all enabled systems                                 */
/*-----------------------------------------------------------------------------*/
void
systemInitAll(void)
{
    system_t *system = systems;
    while (system != NULL && system->init != NULL) {
        if (system->enabled == true) {
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
    system_t *system = systems;
    while (system != NULL && system->init != NULL) {
        if (system->enabled == true) {
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
    system_t *system = systems;
    while (system != NULL && system->init != NULL) {
        if (system->enabled == true) {
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
    system_t *system = systems;
    while (system != NULL && system->init != NULL) {
        if (system->enabled == true) {
            system->unlock();
        }
        system++;
    }
    return;
}
