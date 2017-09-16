// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c et
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     vexuser.c                                                    */
/*    Author:     James Pearman                                                */
/*    Created:    7 May 2013                                                   */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00  04 July 2013 - Initial release                        */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    The author is supplying this software for use with the VEX cortex        */
/*    control system. This file can be freely distributed and teams are        */
/*    authorized to freely use this program , however, it is requested that    */
/*    improvements or additions be shared with the Vex community via the vex   */
/*    forum.  Please acknowledge the work of the authors when appropriate.     */
/*    Thanks.                                                                  */
/*                                                                             */
/*    Licensed under the Apache License, Version 2.0 (the "License");          */
/*    you may not use this file except in compliance with the License.         */
/*    You may obtain a copy of the License at                                  */
/*                                                                             */
/*      http://www.apache.org/licenses/LICENSE-2.0                             */
/*                                                                             */
/*    Unless required by applicable law or agreed to in writing, software      */
/*    distributed under the License is distributed on an "AS IS" BASIS,        */
/*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. */
/*    See the License for the specific language governing permissions and      */
/*    limitations under the License.                                           */
/*                                                                             */
/*    The author can be contacted on the vex forums as jpearman                */
/*    or electronic mail using jbpearman_at_mac_dot_com                        */
/*    Mentor for team 8888 RoboLancers, Pasadena CA.                           */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header
#include "vex.h" // vex library header

#include "smartmotor.h"

// Digital I/O configuration
static vexDigiCfg dConfig[kVexDigital_Num] = {
    {kVexDigital_1, kVexSensorDigitalOutput, kVexConfigOutput, 0}, {kVexDigital_2, kVexSensorDigitalOutput, kVexConfigOutput, 0},
    {kVexDigital_3, kVexSensorDigitalInput, kVexConfigInput, 0},   {kVexDigital_4, kVexSensorDigitalInput, kVexConfigInput, 0},
    {kVexDigital_5, kVexSensorDigitalInput, kVexConfigInput, 0},   {kVexDigital_6, kVexSensorDigitalInput, kVexConfigInput, 0},
    {kVexDigital_7, kVexSensorDigitalInput, kVexConfigInput, 0},   {kVexDigital_8, kVexSensorDigitalInput, kVexConfigInput, 0},
    {kVexDigital_9, kVexSensorDigitalInput, kVexConfigInput, 0},   {kVexDigital_10, kVexSensorDigitalInput, kVexConfigInput, 0},
    {kVexDigital_11, kVexSensorDigitalInput, kVexConfigInput, 0},  {kVexDigital_12, kVexSensorDigitalInput, kVexConfigInput, 0}};

// Port 1 has no power expander
// port 9 SW
// port 2 NE
// Motor configuration
static vexMotorCfg mConfig[kVexMotorNum] = {{kVexMotor_1, kVexMotor393S, kVexMotorNormal, kVexSensorIME, 0},
                                            {kVexMotor_2, kVexMotor393S, kVexMotorNormal, kVexSensorNone, 0},
                                            {kVexMotor_3, kVexMotorUndefined, kVexMotorNormal, kVexSensorNone, 0},
                                            {kVexMotor_4, kVexMotor393S, kVexMotorReversed, kVexSensorNone, 0},
                                            {kVexMotor_5, kVexMotor393T, kVexMotorNormal, kVexSensorIME, kImeChannel_3},
                                            {kVexMotor_6, kVexMotor393S, kVexMotorReversed, kVexSensorNone, 0},
                                            {kVexMotor_7, kVexMotor393T, kVexMotorNormal, kVexSensorIME, kImeChannel_2},
                                            {kVexMotor_8, kVexMotor393S, kVexMotorReversed, kVexSensorIME, kImeChannel_1},
                                            {kVexMotor_9, kVexMotor393S, kVexMotorReversed, kVexSensorNone, 0},
                                            {kVexMotor_10, kVexMotor393S, kVexMotorNormal, kVexSensorIME, 0}};

/*-----------------------------------------------------------------------------*/
/** @brief      User setup                                                     */
/*-----------------------------------------------------------------------------*/
/** @details
 *  The digital and motor ports can (should) be configured here.
 */
void
vexUserSetup()
{
    vexDigitalConfigure(dConfig, DIG_CONFIG_SIZE(dConfig));
    vexMotorConfigure(mConfig, MOT_CONFIG_SIZE(mConfig));
}

/*-----------------------------------------------------------------------------*/
/** @brief      User initialize                                                */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This function is called after all setup is complete and communication has
 *  been established with the master processor.
 *  Start other tasks and initialize user variables here
 */
void
vexUserInit()
{
    // SmartMotorsInit();
    // SmartMotorCurrentMonitorEnable();
    // SmartMotorPtcMonitorEnable();
    // SmartMotorSetPowerExpanderStatusPort(kVexAnalog_3);
    // SmartMotorsAddPowerExtender(kVexMotor_2, kVexMotor_7, kVexMotor_8, kVexMotor_9);
    // SmartMotorRun();
}

/*-----------------------------------------------------------------------------*/
/** @brief      Autonomous                                                     */
/*-----------------------------------------------------------------------------*/

/** @details
 *  This thread is started when the autonomous period is started
 */
msg_t
vexAutonomous(void *arg)
{
    (void)arg;

    // Must call this
    vexTaskRegister("auton");

    // Give the system half a second to restart the LCD
    vexSleep(500);

    return (msg_t)0;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Driver control                                                 */
/*-----------------------------------------------------------------------------*/
/** @details
 *  This thread is started when the driver control period is started
 */
msg_t
vexOperator(void *arg)
{
    // int16_t blink = 0;

    (void)arg;

    // Must call this
    vexTaskRegister("operator");

    // Give the system half a second to restart the LCD
    vexSleep(500);

    // Run until asked to terminate
    while (!chThdShouldTerminate()) {
        // flash led/digi out
        // vexDigitalPinSet( kVexDigital_1, (blink++ >> 3) & 1);

        // Don't hog cpu
        vexSleep(25);
    }

    return (msg_t)0;
}
