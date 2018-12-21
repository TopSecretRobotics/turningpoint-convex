// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*-----------------------------------------------------------------------------*/
/** @file    autonomous/slot7.c                                                */
/** @brief   The autonomous routine 7 for the robot                            */
/*-----------------------------------------------------------------------------*/

#include "autonomous.h"
#include "autonomous/mode.h"

void
autonomousMode7(void)
{
    {
        systemUnlockAll();

        {
            {
                stopMovementOf(ROBOT_ALL, 150);

                // Shoot Flag
                timerRun(2500, { liftMove(127, true); });

                stopMovementOf(ROBOT_ALL, 100);

                timerRun(310, {
                    driveMove(100, 0, true);
                    intakeMove(127, true);
                });

                stopMovementOf(ROBOT_ALL, 75);

                timerRun(220, { driveMove(0, -127, true); });

                // });

                // stopMovementOf(ROBOT_DRIVE, 50);

                // // Back Up
                // timerRun(150, {
                //     driveMove(0, -100, true);
                // });

                stopMovementOf(ROBOT_DRIVE, 50);

                // Go grab ball under Cap
                timerRun(1350, {
                    driveMove(0, 100, true);
                    intakeMove(127, true);
                });

                stopMovementOf(ROBOT_DRIVE, 75);

                // Come back with the ball
                timerRun(1500, {
                    driveMove(-5, -95, true);
                    intakeMove(127, true);
                });

                stopMovementOf(ROBOT_DRIVE, 120);

                // Go forward
                timerRun(95, { driveMove(0, 100, true); });

                stopMovementOf(ROBOT_DRIVE, 50);

                // Turn to shoot
                timerRun(405, {
                    driveMove(-75, 0, true);
                    intakeMove(127, true);
                });

                stopMovementOf(ROBOT_DRIVE, 60);

                timerRun(300, { intakeMove(-127, true); });

                timerRun(200, { intakeMove(127, true); });

                // Shoot at medium
                timerRun(2500, {
                    setterMove(30, true);
                    liftMove(127, true);
                    intakeMove(127, true);
                });
            }
            // Go forward with Drift/ Hit Low flag
            timerRun(2800, {
                driveMove(-12, 127, true);
                // intakeMove(127, true);
            });

            stopMovementOf(ROBOT_ALL, 50);
        }
        return;
    }
}
