// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*-----------------------------------------------------------------------------*/
/** @file    autonomous/slot2.c                                                */
/** @brief   The autonomous routine 2 for the robot                            */
/*-----------------------------------------------------------------------------*/

#include "autonomous.h"
#include "autonomous/mode.h"

void
autonomousMode2(void)
{
    systemUnlockAll();
    // armUnlock();
    // driveUnlock();
    // intakeUnlock();
    // liftUnlock();
    // setterUnlock();

    {
        // Drive Forward and Drop Cone on Mobile Goal

        timerRun(500, {
            setterMove(0, true);
            liftMove(30, true);
            intakeMove(127, true);
            driveMove(0, 100, true);
            armMove(127, true);
        });

        stopMovementOf((ROBOT_DRIVE | ROBOT_ARM), 25);

        timerRun(1425, {
            driveMove(0, 127, true);
            intakeMove(100, true);
            liftMove(30, true);
        });

        stopMovementOf(ROBOT_DRIVE, 800);

        timerRun(200, {
            intakeMove(-127, true);
            /**/
        });

        stopMovementOf(ROBOT_INTAKE, 200);
        stopMovementOf(ROBOT_DRIVE, 200);
    }

    {
        // Backup and Grab the Mobile Base

        timerRun(100, {
            liftMove(60, true);
            /**/
        });

        timerRun(1600, {
            driveMove(0, -60, true);
            setterMove(127, true);
        });

        stopMovementOf((ROBOT_DRIVE | ROBOT_SETTER), 25);

        timerRun(1000, {
            driveMove(0, 127, true);
            /**/
        });

        stopMovementOf(ROBOT_DRIVE, 200);

        timerRun(100, {
            liftMove(127, true);
            /**/

        });

        stopMovementOf(ROBOT_LIFT, 25);

        timerRun(1200, {
            setterMove(-127, true);
            /**/

        });

        stopMovementOf(ROBOT_SETTER, 25);
    }

    return;
}
