// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*-----------------------------------------------------------------------------*/
/** @file    autonomous/slot3.c                                                */
/** @brief   The autonomous routine 3 for the robot                            */
/*-----------------------------------------------------------------------------*/

#include "autonomous.h"
#include "autonomous/mode.h"

void
autonomousMode4(void)
{
    systemUnlockAll();

    {
        {
            stopMovementOf(ROBOT_ALL, 150);

            // Shoot Flag
            timerRun(2500, { liftMove(127, true); });

            stopMovementOf(ROBOT_ALL, 100);

            timerRun(310, {
                driveMove(-100, 0, true);
                intakeMove(127, true);
            });

            stopMovementOf(ROBOT_ALL, 75);

            timerRun(200, { driveMove(0, -127, true); });

            timerRun(30, { driveMove(0, -30, true); });

            // });

            // stopMovementOf(ROBOT_DRIVE, 50);

            // // Back Up
            // timerRun(150, {
            //     driveMove(0, -100, true);
            // });

            stopMovementOf(ROBOT_DRIVE, 50);

            // Go grab ball under Cap
            timerRun(1360, {
                driveMove(5, 100, true);
                intakeMove(127, true);
            });

            stopMovementOf(ROBOT_DRIVE, 75);

            // Come back with the ball
            timerRun(1500, {
                driveMove(-5, -95, true);
                intakeMove(127, true);
            });

            stopMovementOf(ROBOT_DRIVE, 120);

            timerRun(30, { driveMove(7, -30, true); });

            stopMovementOf(ROBOT_DRIVE, 20);

            // Go forward
            timerRun(95, { driveMove(0, 100, true); });

            stopMovementOf(ROBOT_DRIVE, 50);

            // Turn to shoot
            timerRun(635, {
                driveMove(40, 0, true);
                intakeMove(127, true);
            });

            stopMovementOf(ROBOT_DRIVE, 60);

            timerRun(300, { intakeMove(-127, true); });

            timerRun(200, { intakeMove(127, true); });

            // Shoot at medium
            timerRun(2500, {
                setterMove(-30, true);
                liftMove(127, true);
                intakeMove(127, true);
            });
        }
        // Park on 6pt

        //     timerRun(20, { driveMove(127, 0, true); });

        //     stopMovementOf(ROBOT_DRIVE, 150);

        //     timerRun(520, { driveMove(7, -127, true); });

        //     stopMovementOf(ROBOT_DRIVE, 120);

        //     // timerRun(50, {
        //     // liftMove(127, true);
        //     // });

        //     stopMovementOf(ROBOT_ALL, 25);

        //     timerRun(400, { driveMove(-100, 0, true); });

        //     stopMovementOf(ROBOT_DRIVE, 25);

        //     timerRun(300, { driveMove(0, -127, true); });

        //     stopMovementOf(ROBOT_DRIVE, 150);

        //     timerRun(1800, {
        //         driveMove(0, 100, true);
        //         // liftMove(30, true);
        //     });

        //     stopMovementOf(ROBOT_ALL, 1000);
    }

    return;
}
