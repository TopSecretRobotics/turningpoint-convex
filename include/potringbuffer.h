// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
// Copyright (c) 2013-2016 Barobo, Inc.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef _POTRINGBUFFER_H_

#define _POTRINGBUFFER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef SFP_CONFIG_HISTORY_CAPACITY
/* Must be a power of two for use in the ring buffer. */
#define SFP_CONFIG_HISTORY_CAPACITY 8
#endif

#ifndef SFP_CONFIG_MAX_PACKET_SIZE
#define SFP_CONFIG_MAX_PACKET_SIZE 256
#endif

typedef struct SFPpacket {
    uint8_t buf[SFP_CONFIG_MAX_PACKET_SIZE];
    size_t len;
} SFPpacket;

typedef struct PotRingbuffer {
    size_t mBegin;
    size_t mEnd;
    SFPpacket mData[SFP_CONFIG_HISTORY_CAPACITY];
} PotRingbuffer;

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the ringbuffer */
extern void potRingbufferInit(PotRingbuffer *p);
/* Capacity of the ringbuffer */
extern size_t potRingbufferCapacity(PotRingbuffer *p);
/* Number of elements in ringbuffer. */
extern size_t potRingbufferSize(PotRingbuffer *p);
/* True if ringbuffer is empty. */
extern bool potRingbufferEmpty(PotRingbuffer *p);
/* True if ringbuffer is full. */
extern bool potRingbufferFull(PotRingbuffer *p);
/* Array-like access, counting forward from begin. */
extern SFPpacket *potRingbufferAt(PotRingbuffer *p, size_t index);
/* Array-like access, counting backward from end. */
extern SFPpacket *potRingbufferReverseAt(PotRingbuffer *p, size_t index);
/* Access the first element. */
extern SFPpacket *potRingbufferFront(PotRingbuffer *p);
/* Access the last element. */
extern SFPpacket *potRingbufferBack(PotRingbuffer *p);
/* Append an element to the back. */
extern void potRingbufferPushBack(PotRingbuffer *p, const SFPpacket *elem);
/* Prepend an element to the front. */
extern void potRingbufferPushFront(PotRingbuffer *p, const SFPpacket *elem);
/* Remove the first element. */
extern void potRingbufferPopFront(PotRingbuffer *p);
/* Remove the last element. */
extern void potRingbufferPopBack(PotRingbuffer *p);

#ifdef __cplusplus
}
#endif

#endif
