// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*-----------------------------------------------------------------------------*/
/** @file    autonomous/slot3.c                                                */
/** @brief   The autonomous routine 3 for the robot                            */
/*-----------------------------------------------------------------------------*/

#include "autonomous.h"
#include "autonomous/mode.h"

void
autonomousMode3(void)
{
    systemUnlockAll();

    {
        stopMovementOf(ROBOT_ALL, 150);

        // Shoot Flag
        timerRun(2500, { liftMove(127, true); });

        stopMovementOf(ROBOT_ALL, 100);

        // Go forward with Drift/ Hit Low flag
        timerRun(1500, {
            driveMove(18, 127, true);
            // intakeMove(127, true);
        });

        stopMovementOf(ROBOT_ALL, 50);
    }
    {

        // Go back to start
        timerRun(1125, {
            driveMove(-12, -127, true);
            // intakeMove(127, true);
        });

        stopMovementOf(ROBOT_DRIVE, 50);

        // Turn to hit Cap
        timerRun(350, { driveMove(-100, 0, true); });

        stopMovementOf(ROBOT_DRIVE, 50);

        // Back against wall
        timerRun(500, { driveMove(0, -100, true); });

        stopMovementOf(ROBOT_DRIVE, 50);

        // // Go forward
        // timerRun(75, {
        //     driveMove(0, 100, true);
        // });

        // stopMovementOf(ROBOT_DRIVE, 50);

        // // Back Up
        // timerRun(150, {
        //     driveMove(0, -100, true);
        // });

        stopMovementOf(ROBOT_DRIVE, 50);

        // Go grab ball under Cap
        timerRun(1350, {
            driveMove(5, 100, true);
            intakeMove(127, true);
        });

        stopMovementOf(ROBOT_DRIVE, 75);

        // Come back with the ball
        timerRun(1500, {
            driveMove(-10, -100, true);
            intakeMove(127, true);
        });

        // // Go forward
        // timerRun(75, {
        //     driveMove(0, 100, true);
        // });

        // stopMovementOf(ROBOT_DRIVE, 50);

        // // Back Up
        // timerRun(350, {
        //     driveMove(0, -90, true);
        // });

        stopMovementOf(ROBOT_DRIVE, 80);

        // Go forward
        timerRun(95, { driveMove(0, 100, true); });

        stopMovementOf(ROBOT_DRIVE, 50);

        // Turn to shoot
        timerRun(185, {
            driveMove(127, 0, true);
            intakeMove(-127, true);
        });

        stopMovementOf(ROBOT_DRIVE, 60);

        timerRun(100, { intakeMove(127, true); });

        // Shoot at medium
        timerRun(2500, {
            setterMove(30, true);
            liftMove(127, true);
            intakeMove(127, true);
        });
    }
    // Park on 6pt
    {
        stopMovementOf(ROBOT_DRIVE, 150);

        timerRun(390, { driveMove(25, -127, true); });

        stopMovementOf(ROBOT_DRIVE, 100);

        // timerRun(50, {
        // liftMove(127, true);
        // });

        stopMovementOf(ROBOT_ALL, 25);

        timerRun(250, { driveMove(-127, 0, true); });

        stopMovementOf(ROBOT_DRIVE, 25);

        timerRun(250, { driveMove(0, -127, true); });

        stopMovementOf(ROBOT_DRIVE, 100);
        timerRun(1800, {
            driveMove(0, 100, true);
            // liftMove(30, true);
        });

        stopMovementOf(ROBOT_ALL, 1000);
    }

    return;
}
