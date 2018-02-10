// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*-----------------------------------------------------------------------------*/
/** @file    autonomous/mode0.c                                                */
/** @brief   The autonomous routine 0 for the robot                            */
/*-----------------------------------------------------------------------------*/

#include "autonomous.h"
#include "autonomous/mode.h"

/**
 * ██╗  ██╗███████╗ █████╗ ██████╗ ███████╗    ██╗   ██╗██████╗ ██╗
 * ██║  ██║██╔════╝██╔══██╗██╔══██╗██╔════╝    ██║   ██║██╔══██╗██║
 * ███████║█████╗  ███████║██║  ██║███████╗    ██║   ██║██████╔╝██║
 * ██╔══██║██╔══╝  ██╔══██║██║  ██║╚════██║    ██║   ██║██╔═══╝ ╚═╝
 * ██║  ██║███████╗██║  ██║██████╔╝███████║    ╚██████╔╝██║     ██╗
 * ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═════╝ ╚══════╝     ╚═════╝ ╚═╝     ╚═╝
 *
 * Read the comments below carefully, I've tried to document how each
 * of the "helper" functions are designed to be used.
 *
 */

void
autonomousMode0(void)

// Place preload on Tower
{
    // Unlocking disables PID and operator control for a component
    systemUnlockAll();
    // armUnlock();
    // driveUnlock();
    // intakeUnlock();
    // liftUnlock();
    // setterUnlock();

    // Place pre-load on the tower
    {
        // stopMovement(50);
        timerRun(100, {
            liftMove(65, true);
            armMove(35, true);
            intakeMove(127, true);
        });

        stopMovementOf(ROBOT_ALL, 25);

        timerRun(200, {
            liftMove(-65, true);
            intakeMove(127, true);
        });

        stopMovementOf(ROBOT_ALL, 25);

        // timerRun(200, {

        timerRun(1000, { intakeMove(25, true); });

        stopMovementOf(ROBOT_ALL, 50);

        timerRun(20, {
            armMove(127, true);
            liftMove(127, true);
            intakeMove(50, true);
        });

        stopMovementOf(ROBOT_ALL, 50);

        // timerRun(250, {
        //     liftMove(127, true);
        //     intakeMove(127, true);
        // });

        timerRun(250, { armMove(127, true); });

        stopMovementOf(ROBOT_ALL, 50);

        timerRun(3400, { driveMove(0, 127, true); });

        stopMovementOf(ROBOT_ALL, 50);

        // timerRun(500,{
        // driveMove(0, -127, true);
        // });

        // timerRun(2500, {
        // setterMove(127, true);
        // });

        // timerRun(1000, {
        //     intakeMove(127, true);
        // });

        // timerRun(900, {
        //     intakeMove(127, true);
        //     armMove(100, true);
        // });

        // timerRun()
    }
    return;
}