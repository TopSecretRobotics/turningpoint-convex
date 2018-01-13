// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*
 * lcd.h
 */

#ifndef LCD_H_

#define LCD_H_

#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header
#include "vex.h" // vex library header

#include "vexflash.h" // vex flash user parameter

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    kLcdModeSetup = -1,
    kLcdMode0 = 0,
    kLcdMode1,
    kLcdMode2,
    kLcdMode3,
    kLcdMode4,
    kLcdMode5,
    kLcdMode6,
    kLcdMode7,
    kLcdMode8,
    kLcdMode9,
    kLcdModeNumber
} kLcdModeType;

typedef struct lcd_s {
    uint8_t display;
    kLcdModeType mode;
    vexLcdButton buttons;
} lcd_t;

extern lcd_t *lcdGetPtr(void);
extern void lcdSetup(uint8_t display);
extern void lcdInit(void);
extern void lcdStart(void);
extern kLcdModeType lcdGetMode(void);

#ifdef __cplusplus
}
#endif

#endif
