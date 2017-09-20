// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
// Copyright (c) 2013-2016 Barobo, Inc.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "potringbuffer.h"

#include <string.h>

// private functions
static SFPpacket *potRingbufferWrappedAccess(PotRingbuffer *p, size_t index);
static void potRingbufferAdd(PotRingbuffer *p, size_t *beginOrEnd, size_t amount);
static void potRingbufferIncr(PotRingbuffer *p, size_t *beginOrEnd);
static void potRingbufferDecr(PotRingbuffer *p, size_t *beginOrEnd);

// public functions

/* Initialize the ringbuffer */
void
potRingbufferInit(PotRingbuffer *p)
{
    p->mBegin = 0;
    p->mEnd = 0;
}

/* Capacity of the ringbuffer */
size_t
potRingbufferCapacity(PotRingbuffer *p)
{
    (void)p;
    return SFP_CONFIG_HISTORY_CAPACITY;
}

/* Number of elements in ringbuffer. */
size_t
potRingbufferSize(PotRingbuffer *p)
{
    return (potRingbufferFull(p)) ? SFP_CONFIG_HISTORY_CAPACITY : (p->mEnd - p->mBegin) & (SFP_CONFIG_HISTORY_CAPACITY - 1);
}

/* True if ringbuffer is empty. */
bool
potRingbufferEmpty(PotRingbuffer *p)
{
    return (p->mBegin == p->mEnd);
}

/* True if ringbuffer is full. */
bool
potRingbufferFull(PotRingbuffer *p)
{
    return ((p->mBegin ^ SFP_CONFIG_HISTORY_CAPACITY) == p->mEnd);
}

/* Array-like access, counting forward from begin. */
SFPpacket *
potRingbufferAt(PotRingbuffer *p, size_t index)
{
    return potRingbufferWrappedAccess(p, p->mBegin + index);
}

/* Array-like access, counting backward from end. */
SFPpacket *
potRingbufferReverseAt(PotRingbuffer *p, size_t index)
{
    return potRingbufferWrappedAccess(p, p->mEnd - index);
}

/* Access the first element. */
SFPpacket *
potRingbufferFront(PotRingbuffer *p)
{
    return potRingbufferAt(p, 0);
}

/* Access the last element. */
SFPpacket *
potRingbufferBack(PotRingbuffer *p)
{
    return potRingbufferReverseAt(p, 1);
}

/* Append an element to the back. */
void
potRingbufferPushBack(PotRingbuffer *p, const SFPpacket *elem)
{
    if (potRingbufferFull(p)) {
        potRingbufferIncr(p, &(p->mBegin));
    }
    potRingbufferIncr(p, &(p->mEnd));
    memcpy(potRingbufferBack(p), elem, sizeof(SFPpacket));
}

/* Prepend an element to the front. */
void
potRingbufferPushFront(PotRingbuffer *p, const SFPpacket *elem)
{
    if (potRingbufferFull(p)) {
        potRingbufferDecr(p, &(p->mEnd));
    }
    potRingbufferDecr(p, &(p->mBegin));
    memcpy(potRingbufferFront(p), elem, sizeof(SFPpacket));
}

/* Remove the first element. */
void
potRingbufferPopFront(PotRingbuffer *p)
{
    potRingbufferIncr(p, &(p->mBegin));
}

/* Remove the last element. */
void
potRingbufferPopBack(PotRingbuffer *p)
{
    potRingbufferDecr(p, &(p->mEnd));
}

// private functions

static SFPpacket *
potRingbufferWrappedAccess(PotRingbuffer *p, size_t index)
{
    return &(p->mData[index & (SFP_CONFIG_HISTORY_CAPACITY - 1)]);
}

static void
potRingbufferAdd(PotRingbuffer *p, size_t *beginOrEnd, size_t amount)
{
    (void)p;
    *beginOrEnd = (*beginOrEnd + amount) & (2 * SFP_CONFIG_HISTORY_CAPACITY - 1);
}

static void
potRingbufferIncr(PotRingbuffer *p, size_t *beginOrEnd)
{
    potRingbufferAdd(p, beginOrEnd, 1);
}

static void
potRingbufferDecr(PotRingbuffer *p, size_t *beginOrEnd)
{
    potRingbufferAdd(p, beginOrEnd, -1);
}
