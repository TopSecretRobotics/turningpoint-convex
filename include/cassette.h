// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*
 * cassette.h
 */

#ifndef CASSETTE_H_

#define CASSETTE_H_

#include <stdint.h>
#include <string.h>

#include "vexflash.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t cassetteCount(void);
extern uint8_t cassetteFree(void);
extern uint8_t cassetteMax(void);
extern user_param *cassetteOpenWrite(uint8_t index);
extern user_param *cassetteOpenRead(uint8_t index);

#ifdef __cplusplus
}
#endif

#endif
