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

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#if BYTE_ORDER == LITTLE_ENDIAN

#define HTONS(n) __builtin_bswap16(n)
#define NTOHS(n) __builtin_bswap16(n)
#define HTONL(n) __builtin_bswap32(n)
#define NTOHL(n) __builtin_bswap32(n)
#define HTONLL(n) __builtin_bswap64(n)
#define NTOHLL(n) __builtin_bswap64(n)

#else

#define HTONS(n) (n)
#define NTOHS(n) (n)
#define HTONL(n) (n)
#define NTOHL(n) (n)
#define HTONLL(n) (n)
#define NTOHLL(n) (n)

#endif

#define htons(n) HTONS(n)
#define ntohs(n) NTOHS(n)
#define htonl(n) HTONL(n)
#define ntohl(n) NTOHL(n)
#define htonll(n) HTONLL(n)
#define ntohll(n) NTOHLL(n)

#endif
