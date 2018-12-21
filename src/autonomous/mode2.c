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
  * Read the comments below carefully, I've tried to document how each
 * of the "helper" functions are designed to be used.
 *
 */

void
autonomousMode2(void)

{
    systemUnlockAll();

    // Park on 6pt
    {
        stopMovementOf(ROBOT_DRIVE, 150);

        timerRun(375, { driveMove(0, 127, true); });

        stopMovementOf(ROBOT_DRIVE, 100);

        // timerRun(50, {
        // liftMove(127, true);
        // });

        stopMovementOf(ROBOT_ALL, 25);

        timerRun(250, { driveMove(127, 0, true); });

        stopMovementOf(ROBOT_DRIVE, 25);

        timerRun(250, { driveMove(0, -127, true); });

        stopMovementOf(ROBOT_DRIVE, 100);
        timerRun(2800, {
            driveMove(0, 100, true);
            // liftMove(30, true);
        });

        stopMovementOf(ROBOT_ALL, 1000);
    }

    return;
}

/* ---------------------------------------------------------------- */
/* Drive forward and push setter out, hold preload ------- 49 - 85  */
/* ---------------------------------------------------------------- */
/* Bring mobile goal in and score pre-load --------------- 87 - 113 */
/* ---------------------------------------------------------------- */
/*   */
/* ---------------------------------------------------------------- */
/*   */
/* ---------------------------------------------------------------- */
/*   */
/* ---------------------------------------------------------------- */

// {
//     systemUnlockAll();
//     // armUnlock();
//     // driveUnlock();
//     // intakeUnlock();
//     // liftUnlock();
//     // setterUnlock();

//     {
//         // Raise lift & pull pre-load cone in (1);

//         timerRun(320, {
//             liftMove(127, true);
//             intakeMove(127, true);
//             driveMove(0, 0, true);
//         });

//         stopMovementOf(ROBOT_LIFT, 25);

//         // Put setter out & pull pre-load cone in (2);

//         timerRun(1200, {
//             liftMove(25, true);
//             setterMove(127, true);
//             intakeMove(100, true);
//             driveMove(0, 0, true);
//         });

//         stopMovementOf((ROBOT_SETTER | ROBOT_DRIVE | ROBOT_LIFT), 25);

//         // Drive forward & pull pre-load cone in (3);

//         timerRun(400, {
//             liftMove(25, true);
//             intakeMove(100, true);
//             driveMove(10, 100, true);
//         });

//         stopMovementOf((ROBOT_DRIVE | ROBOT_LIFT), 25);

//         // Drive forward & grab mobile goal, & pull pre-load cone in (4);

//         timerRun(1750, {
//             driveMove(0, 127, true);
//             intakeMove(100, true);
//         });

//         stopMovementOf((ROBOT_DRIVE | ROBOT_SETTER), 25);
//     }

//     {
//         // Bring mobile goal in the robot

//         timerRun(1400, {
//             setterMove(-127, true);
//             /**/
//         });

//         stopMovementOf(ROBOT_SETTER, 25);

//     //     // Drop lift

//         timerRun(300, {
//             liftMove(-127, true);
//             /**/
//         });
//         stopMovementOf(ROBOT_LIFT, 25);

//         // Drop pre-load cone (1) onto the mobile goal

//         timerRun(100, {
//             intakeMove(-127, true);
//             /**/
//         });

//         stopMovementOf((ROBOT_LIFT | ROBOT_INTAKE), 25);

//         // Release pre-load cone (1) & raise lift

//         timerRun(375, {
//             liftMove(127, true);
//             // intakeMove(-127, true);
//         });

//         stopMovementOf(ROBOT_LIFT, 25);
//     }

//     {
//         // Second cone

//         // raise arm

//         timerRun(250, {
//             armMove(127, true);
//             /**/
//         });

//         stopMovementOf((ROBOT_LIFT | ROBOT_ARM), 25);

//         // Drop lift, suck intake in & drive forward onto the cone (2)

//         timerRun(560, {
//             liftMove(-100, true);
//             intakeMove(127, true);
//             driveMove(0, 70, true);
//         });

//         stopMovementOf(ROBOT_DRIVE, 75);

//         // drive forward & grab cone (2)

//         timerRun(540, {
//             liftMove(-100, true);
//             intakeMove(127, true);
//             driveMove(0, 100, true);
//         });

//         stopMovementOf(ROBOT_DRIVE, 25);

//         // Raise lift, & pull arm in & hold cone (2)

//         timerRun(450, {
//             liftMove(100, true);
//             intakeMove(100, true);
//             armMove(-127, true);
//         });

//         stopMovementOf(ROBOT_LIFT, 25);

//         // Lower lift onto stack & hold cone (2)

//         timerRun(450, {
//             liftMove(-127, true);
//             intakeMove(100, true);
//         });

//         // Raise lift & release cone

//         timerRun(450, {
//             liftMove(127, true);
//             intakeMove(-127, true);
//             // armMove(127, true);
//         });

//         stopMovementOf((ROBOT_INTAKE | ROBOT_LIFT), 25);
//     }

//     {
//         // Third cone

//         // // push arm out

//         // timerRun(250, {
//         //     armMove(127, true);
//         //     /**/
//         // });

//         // stopMovementOf((ROBOT_LIFT | ROBOT_ARM), 25);

//         // // Lower lift, drive forward & grab cone (3)

//         // timerRun(600, {
//         //     liftMove(-100, true);
//         //     intakeMove(127, true);
//         //     driveMove(0, 80, true);
//         // });

//         // stopMovementOf(ROBOT_ALL, 25);

//         // // drive forward & grab cone (3)

//         // timerRun(500, {
//         //     intakeMove(127, true);
//         //     driveMove(20, 80, true);
//         // });

//         // stopMovementOf(ROBOT_DRIVE, 25);

//         // // Raise lift, pull arm in & hold cone (3)

//         // timerRun(450, {
//         //     liftMove(127, true);
//         //     intakeMove(127, true);
//         //     armMove(-100, true);
//         // });

//         // stopMovementOf(ROBOT_LIFT, 25);

//         // // lower lift & hold cone (3)

//         // timerRun(350, {
//         //     liftMove(-90, true);
//         //     intakeMove(100, true);
//         // });

//         // stopMovementOf(ROBOT_LIFT, 25);

//         // // raise lift & release cone

//         // timerRun(300, {
//         //     intakeMove(-127, true);
//         //     liftMove(127, true);
//         // });

//         // stopMovementOf(ROBOT_LIFT, 25);
//     }

//     {
//         // drive back to stack

//         // drive backward to start (1)

//         timerRun(1900, {
//             driveMove(20, -127, true);
//             liftMove(30, true);
//             /**/
//         });

//         stopMovementOf(ROBOT_ALL, 25);

//         timerRun(200000, {
//             liftMove(20, true);
//         });

//         // turn right

//         timerRun(500, {
//             driveMove(-127, 0, true);
//             /**/
//         });

//         // stopMovementOf(ROBOT_DRIVE, 25);

//         // // drive backward

//         // timerRun(800, {
//         //     driveMove(0, -127, true);
//         //     /**/
//         // });

//         // stopMovementOf(ROBOT_DRIVE, 25);

//         // // turn left

//         // timerRun(570, {
//         //     driveMove(127, 0, true);
//         //     /**/
//         // });

//         // stopMovementOf(ROBOT_DRIVE, 25);

//         // // drive forward

//         // timerRun(1150, {
//         //     liftMove(20, true);
//         //     driveMove(0, 127, true);
//         //     setterMove(127, true);
//         // });

//         // stopMovementOf(ROBOT_DRIVE, 25);

//         // // Push setter out

//         // timerRun(700, {
//         //     setterMove(127, true);
//         //     /**/
//         // });

//         // // move setter out

//         // // timerRun(1200, {
//         // // liftMove(20, true);
//         // // setterMove(127, true);

//         // // });

//         // stopMovementOf(ROBOT_ALL, 25);

//         // timerRun(1000, {
//         //     driveMove(-25, -127, true);
//         //     setterMove(-127, true);
//         // });

//         // stopMovementOf(ROBOT_DRIVE, 25);

//         // // timerRun(2000, {
//         // // liftMove(20, true);
//         // // });
//         // //
//         // // stopMovementOf(ROBOT_ALL, 25);

//         // timerRun(540, {
//         //     driveMove(-127, 0, true);
//         //     /**/
//         // });

//         // stopMovementOf(ROBOT_DRIVE, 25);

//         // timerRun(500, {
//         //     driveMove(0, 127, true);
//         //     /**/
//         // });

//         // stopMovementOf(ROBOT_DRIVE, 25);

//         // timerRun(560, {
//         //     driveMove(-127, 0, true);
//         //     /**/
//         // });

//         // stopMovementOf(ROBOT_DRIVE, 25);

//         // timerRun(1000, {
//         //     driveMove(0, 127, true);
//         //     /**/
//         // });

//         // stopMovementOf(ROBOT_ALL, 1000);
//     }

//     return;
// }