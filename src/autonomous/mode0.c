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
{
    // Unlocking disables PID and operator control for a component
    systemUnlockAll();
    // armUnlock();
    // driveUnlock();
    // intakeUnlock();
    // liftUnlock();
    // setterUnlock();
    return;
}
