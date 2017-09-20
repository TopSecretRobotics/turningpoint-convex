// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c et
/*
 * rpi3.h
 */

#ifndef RPI3_H_

#define RPI3_H_

#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header
#include "vex.h" // vex library header

#ifdef __cplusplus
extern "C" {
#endif

extern void rpi3Setup(SerialDriver *sd);
extern void rpi3Init(void);
extern void rpi3Start(void);
extern int rpi3IsConnected(void);

#ifdef __cplusplus
}
#endif

#endif
