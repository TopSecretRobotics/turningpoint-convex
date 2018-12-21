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
#include "vexgyro.h"

#include "lcd.h"

#include "arm.h"
#include "drive.h"
#include "intake.h"
#include "lift.h"
#include "setter.h"
#include "flipper.h"

#include "system.h"

#include "autonomous.h"

// #include "server.h"

// Digital I/O configuration
static vexDigiCfg dConfig[kVexDigital_Num] = {{kVexDigital_1, kVexSensorDigitalOutput, kVexConfigOutput, 0},
                                              {kVexDigital_2, kVexSensorDigitalOutput, kVexConfigOutput, 0},
                                              {kVexDigital_3, kVexSensorDigitalInput, kVexConfigInput, 0},
                                              {kVexDigital_4, kVexSensorDigitalInput, kVexConfigInput, 0},
                                              {kVexDigital_5, kVexSensorDigitalInput, kVexConfigInput, 0},
                                              {kVexDigital_6, kVexSensorDigitalInput, kVexConfigInput, 0},
                                              {kVexDigital_7, kVexSensorDigitalInput, kVexConfigInput, 0},
                                              {kVexDigital_8, kVexSensorDigitalInput, kVexConfigInput, 0},
                                              {kVexDigital_9, kVexSensorDigitalInput, kVexConfigInput, 0},
                                              {kVexDigital_10, kVexSensorDigitalInput, kVexConfigInput, 0},
                                              {kVexDigital_11, kVexSensorQuadEncoder, kVexConfigQuadEnc1, kVexQuadEncoder_1},
                                              {kVexDigital_12, kVexSensorQuadEncoder, kVexConfigQuadEnc2, kVexQuadEncoder_2}};

// Port 1 has no power expander
// port 9 SW
// port 2 NE                                          36 inches tall 30 inches deep 49 inches wide
// Motor configuration
static vexMotorCfg mConfig[kVexMotorNum] = {{kVexMotor_1, kVexMotor393T, kVexMotorNormal, kVexSensorNone, 0},
                                            {kVexMotor_2, kVexMotor393T, kVexMotorReversed, kVexSensorNone, 0},
                                            {kVexMotor_3, kVexMotor393T, kVexMotorReversed, kVexSensorNone, 0},
                                            {kVexMotor_4, kVexMotor393T, kVexMotorReversed, kVexSensorNone, 0},
                                            {kVexMotor_5, kVexMotor393S, kVexMotorReversed, kVexSensorNone, 0},
                                            {kVexMotor_6, kVexMotor393T, kVexMotorNormal, kVexSensorNone, 0},
                                            {kVexMotor_7, kVexMotor393R, kVexMotorNormal, kVexSensorNone, 0},
                                            {kVexMotor_8, kVexMotor393T, kVexMotorReversed, kVexSensorNone, 0},
                                            {kVexMotor_9, kVexMotor393T, kVexMotorNormal, kVexSensorNone, 0},
                                            {kVexMotor_10, kVexMotor393T, kVexMotorNormal, kVexSensorNone, 0}};

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
    lcdSetup(VEX_LCD_DISPLAY_1);
    // serverSetup(&SD3);
    armSetup(kVexMotor_3 // arm motor
             );
    driveSetup(kVexMotor_2, // drive northeast or front-right motor
               kVexMotor_9, // drive northwest or front-left motor
               kVexMotor_8, // drive southeast or back-right motor
               kVexMotor_1  // drive southwest or back-left motor
               );
    flipperSetup(kVexMotor_4 // flipper motor
                 );
    intakeSetup(kVexMotor_7 // intake motor
                );
    liftSetup(kVexMotor_10, // lift first motor
              kVexMotor_6,  // lift second motor
              kVexAnalog_2, // lift potentiometer
              true,         // reversed potentiometer (values decrease with positive motor speed)
              (1.0 / 7.0),  // gear ratio (1:7 or ~857 ticks per rotation)
              3030,         // floor potentiometer value
              1080          // ceiling potentiometer value
              );
    setterSetup(kVexMotor_5 // setter motor
                );
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
    SmartMotorsInit();
    SmartMotorCurrentMonitorEnable();
    // SmartMotorPtcMonitorEnable();
    SmartMotorSetPowerExpanderStatusPort(kVexAnalog_1);
    SmartMotorsAddPowerExtender(kVexMotor_6, kVexMotor_7, kVexMotor_8, kVexMotor_9);
    systemInitAll();
    SmartMotorRun();
    // serverInit();
    // serverStart();
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

    vexLcdClearLine(VEX_LCD_DISPLAY_1, VEX_LCD_LINE_1);
    vexLcdClearLine(VEX_LCD_DISPLAY_1, VEX_LCD_LINE_2);

    // Give the system half a second to restart the LCD
    vexSleep(500);

    systemStartAll();
    systemLockAll();

    while (1) {
        switch (lcdGetMode()) {
        case kLcdMode0:
            autonomousMode0();
            break;
        case kLcdMode1:
            autonomousMode1();
            break;
        case kLcdMode2:
            autonomousMode2();
            break;
        case kLcdMode3:
            autonomousMode3();
            break;
        case kLcdMode4:
            autonomousMode4();
            break;
        case kLcdMode5:
            autonomousMode5();
            break;
        case kLcdMode6:
            autonomousMode6();
            break;
        case kLcdMode7:
            autonomousMode7();
            break;
        case kLcdMode8:
            autonomousMode8();
            break;
        case kLcdMode9:
            autonomousMode9();
            break;
        default:
            vexSleep(25); // wait 25ms before retry
            break;
        }
        break;
    }

    systemLockAll();

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

    // vexGyroInit(kVexAnalog_6);

    vexLcdClearLine(VEX_LCD_DISPLAY_1, VEX_LCD_LINE_1);
    vexLcdClearLine(VEX_LCD_DISPLAY_1, VEX_LCD_LINE_2);

    // Give the system half a second to restart the LCD
    vexSleep(500);

    systemStartAll();
    systemLockAll();

    // vexLcdButton buttons;
    // serverIpv4_t ipv4;

    // int count = 0;
    // int ret;
    // char *message = "hello\n";

    // int16_t liftCmd = 0;

    // Run until asked to terminate
    while (!chThdShouldTerminate()) {
        // liftCmd = vexControllerGet(Ch3);
        // if (liftCmd > -20 && liftCmd < 20) {
        //     liftCmd = 0;
        // }
        // SetMotor(kVexMotor_4, liftCmd, true);
        // SetMotor(kVexMotor_6, liftCmd, true);
        // SetMotor(kVexMotor_7, vexControllerGet(Ch3), false);
        // SetMotor(kVexMotor_6, liftCmd, true);
        // SetMotor(kVexMotor_10, liftCmd, true);
        // vexMotorSet(kVexMotor_6, vexControllerGet(Ch2));
        // flash led/digi out
        // vexDigitalPinSet( kVexDigital_1, (blink++ >> 3) & 1);

        // buttons = vexLcdButtonGet(VEX_LCD_DISPLAY_1);

        // if (buttons == kLcdButtonRight) {
        //     vexGyroReset();
        //     do {
        //         buttons = vexLcdButtonGet(VEX_LCD_DISPLAY_1);
        //         vexSleep(25);
        //     } while (buttons != kLcdButtonNone);
        // }

        // // status on LCD of server connection
        // vexLcdPrintf(VEX_LCD_DISPLAY_1, VEX_LCD_LINE_1, "%s", serverIsConnected() ? "CONNECTED" : "DISCONNECTED");
        // ipv4 = serverGetIpv4();
        // if (ipv4.v[0] == 0) {
        //     vexLcdClearLine(VEX_LCD_DISPLAY_1, VEX_LCD_LINE_2);
        // } else {
        //     vexLcdPrintf(VEX_LCD_DISPLAY_1, VEX_LCD_LINE_2, "%u.%u.%u.%u", ipv4.v[0], ipv4.v[1], ipv4.v[2], ipv4.v[3]);
        // }

        // status on LCD of encoder and sonar
        // vexLcdPrintf(VEX_LCD_DISPLAY_1, VEX_LCD_LINE_1, "%4.2fV G %6.2f", vexSpiGetMainBattery() / 1000.0, vexGyroGet() / 10.0);

        // if (++count == 100) {
        //     sendControl.frame = YAHDLC_FRAME_DATA;
        //     sendControl.seq_no = seq_no++;
        //     if (seq_no > 7) {
        //         seq_no = 0;
        //     }
        //     ret = yahdlc_frame_data(&sendControl, NULL, 0, sendFrame, &sendLength);
        //     // vexLcdPrintf(VEX_LCD_DISPLAY_1, VEX_LCD_LINE_1, "ret = %d, sl = %d", ret, sendLength);
        //     // vexLcdPrintf(VEX_LCD_DISPLAY_1, VEX_LCD_LINE_2, "seq_no = %d", (int)sendControl.seq_no);
        //     sdWrite(&SD3, (unsigned char *)sendFrame, sendLength);
        //     // sdWrite(&SD3, (unsigned char *)message, 6);
        //     count = 0;
        // }

        // Don't hog cpu
        vexSleep(25);
    }

    return (msg_t)0;
}
