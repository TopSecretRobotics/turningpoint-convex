// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c et
/*
 * portable_endian.h
 */

#ifndef PORTABLE_ENDIAN_H_

#define PORTABLE_ENDIAN_H_

#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header
#include "vex.h" // vex library header

// #if BYTE_ORDER == BIG_ENDIAN

// #define HTONS(n) (n)
// #define NTOHS(n) (n)
// #define HTONL(n) (n)
// #define NTOHL(n) (n)

// #else

#define HTONS(n) (((((unsigned short)(n)&0xFF)) << 8) | (((unsigned short)(n)&0xFF00) >> 8))
#define NTOHS(n) (((((unsigned short)(n)&0xFF)) << 8) | (((unsigned short)(n)&0xFF00) >> 8))

#define HTONL(n)                                                                                                                   \
    (((((unsigned long)(n)&0xFF)) << 24) | ((((unsigned long)(n)&0xFF00)) << 8) | ((((unsigned long)(n)&0xFF0000)) >> 8) |         \
     ((((unsigned long)(n)&0xFF000000)) >> 24))

#define NTOHL(n)                                                                                                                   \
    (((((unsigned long)(n)&0xFF)) << 24) | ((((unsigned long)(n)&0xFF00)) << 8) | ((((unsigned long)(n)&0xFF0000)) >> 8) |         \
     ((((unsigned long)(n)&0xFF000000)) >> 24))
// #endif

#define htons(n) HTONS(n)
#define ntohs(n) NTOHS(n)

#define htonl(n) HTONL(n)
#define ntohl(n) NTOHL(n)

#endif