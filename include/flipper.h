/*
    flipper
*/

#ifndef FLIPPER_H_

#define FLIPPER_H_

#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header
#include "vex.h" // vex library header

#include "pidlib.h"
#include "smartmotor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct flipper_s {
    tVexMotor motor;
    bool locked;
} flipper_t;

extern flipper_t *flipperGetPtr(void);
extern void flipperSetup(tVexMotor motor);
extern void flipperInit(void);
extern void flipperStart(void);
extern void flipperMove(int16_t cmd, bool immediate);
extern void flipperLock(void);
extern void flipperUnlock(void);

#ifdef __cplusplus
}
#endif

#endif
