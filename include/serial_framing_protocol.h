// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
// Copyright (c) 2013-2016 Barobo, Inc.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef _SERIAL_FRAMING_PROTOCOL_H_

#define _SERIAL_FRAMING_PROTOCOL_H_

#include "potringbuffer.h"

#include <stdlib.h>
#include <stdint.h>

#ifndef SFP_CONFIG_WRITEBUF_SIZE
#define SFP_CONFIG_WRITEBUF_SIZE 512
#endif

typedef uint8_t SFPseq;
typedef uint8_t SFPheader;
typedef uint16_t SFPcrc;

/* SFP reserved octets */
enum { SFP_ESC = 0x7d, SFP_FLAG = 0x7e };

#define SFP_ESC_FLIP_BIT (1 << 5) // bit 5, like in HDLC

#define SFP_CRC_SIZE sizeof(SFPcrc)
#define SFP_CRC_PRESET                                                                                                             \
    0xffff /* The initial value for the CRC, recommended \ \ \ \ \ \ \ \ \ \                                                                                                                                 \
            * by an article in Dr. Dobb's Journal */

#define SFP_CRC_GOOD                                                                                                               \
    0xf0b8 /* A CRC updated over its bitwise complement, \ \ \ \ \ \ \ \ \ \                                                                                                                                 \
            * least significant byte first, results in \ \ \ \ \ \ \ \ \ \                                                                                                                                 \
            * this value. */

/* Header format:
 *
 * ccss ssss
 *
 * where cc are the control bits (the frame type), and ss ssss are the
 * sequence number bits. */

#define SFP_FIRST_SEQ_BIT 0
#define SFP_NUM_SEQ_BITS 6
#define SFP_FIRST_CONTROL_BIT SFP_NUM_SEQ_BITS
#define SFP_NUM_CONTROL_BITS 2

#define SFP_SEQ_RANGE (1 << SFP_NUM_SEQ_BITS)
#define SFP_INITIAL_SEQ 0

typedef enum { SFP_FRAME_USR = 0, SFP_FRAME_RTX, SFP_FRAME_NAK, SFP_FRAME_SYN } SFPframetype;

enum { SFP_SEQ_SYN0 = 0, SFP_SEQ_SYN1, SFP_SEQ_SYN2, SFP_SEQ_SYN_DIS };

typedef void (*SFPdeliverfun)(uint8_t *buf, size_t len, void *userdata);
typedef int (*SFPwritefun)(uint8_t *octets, size_t len, size_t *outlen, void *userdata);

typedef enum { SFP_ESCAPE_STATE_NORMAL, SFP_ESCAPE_STATE_ESCAPING } SFPescapestate;

typedef enum { SFP_FRAME_STATE_NEW, SFP_FRAME_STATE_RECEIVING } SFPframestate;

typedef enum {
    SFP_CONNECT_STATE_DISCONNECTED,
    SFP_CONNECT_STATE_SENT_SYN0,
    SFP_CONNECT_STATE_SENT_SYN1,
    SFP_CONNECT_STATE_CONNECTED
} SFPconnectstate;

typedef struct SFPtransmitter {
    SFPseq seq;
    SFPcrc crc;

    PotRingbuffer history;

    uint8_t writebuf[SFP_CONFIG_WRITEBUF_SIZE];
    size_t writebufn;

    SFPwritefun write;
    void *writeData;
} SFPtransmitter;

typedef struct SFPreceiver {
    SFPseq seq;
    SFPcrc crc;

    SFPescapestate escapeState;
    SFPframestate frameState;

    SFPheader header;
    SFPpacket packet;

    SFPdeliverfun deliver;
    void *deliverData;
} SFPreceiver;

typedef struct SFPcontext {
    SFPtransmitter tx;
    SFPreceiver rx;

    SFPconnectstate connectState;
} SFPcontext;

#ifdef __cplusplus
extern "C" {
#endif

/* Return 1 on packet available, 0 on unavailable, -1 on error. */
extern int sfpDeliverOctet(SFPcontext *ctx, uint8_t octet, uint8_t *buf, size_t len, size_t *outlen);
extern int sfpWritePacket(SFPcontext *ctx, const uint8_t *buf, size_t len, size_t *outlen);
extern void sfpConnect(SFPcontext *ctx);
extern int sfpIsConnected(SFPcontext *ctx);

extern size_t sfpGetSizeof(void);
extern void sfpInit(SFPcontext *ctx);

extern void sfpSetDeliverCallback(SFPcontext *ctx, SFPdeliverfun cbfun, void *userdata);
extern void sfpSetWriteCallback(SFPcontext *ctx, SFPwritefun cbfun, void *userdata);

#ifdef __cplusplus
}
#endif

#endif
