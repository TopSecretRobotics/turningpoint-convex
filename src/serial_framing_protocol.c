// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
// Copyright (c) 2013-2016 Barobo, Inc.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "serial_framing_protocol.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////////

/* Stolen from avr-libc's docs */
static uint16_t
_crc_ccitt_update(uint16_t crc, uint8_t octet)
{
    octet ^= crc & 0xff;
    octet ^= octet << 4;
    return ((((uint16_t)octet << 8) | ((crc >> 8) & 0xff)) ^ (uint8_t)(octet >> 4) ^ ((uint16_t)octet << 3));
}

//////////////////////////////////////////////////////////////////////////////

static int isReservedOctet(uint8_t octet);
static SFPseq nextSeq(SFPseq seq);
static SFPframetype getFrameType(SFPheader header);
static SFPseq getFrameSeq(SFPheader header);

//////////////////////////////////////////////////////////////////////////////

static int sfpBufferedWrite(uint8_t octet, size_t *outlen, void *ctx);
static void sfpFlushWriteBuffer(SFPcontext *ctx);

static void sfpClearHistory(SFPcontext *ctx);
static int sfpTransmitFrameWithHeader(SFPcontext *ctx, SFPheader header, SFPpacket *packet, size_t *outlen);
static int sfpTransmitFrameImpl(SFPcontext *ctx, SFPpacket *packet, size_t *outlen, int retransmit);
static void sfpTransmitDIS(SFPcontext *ctx);
static void sfpTransmitSYN0(SFPcontext *ctx);
static void sfpTransmitSYN1(SFPcontext *ctx);
static void sfpTransmitSYN2(SFPcontext *ctx);
static void sfpTransmitNAK(SFPcontext *ctx, SFPseq seq);
static int sfpTransmitUSR(SFPcontext *ctx, SFPpacket *packet, size_t *outlen);
static void sfpTransmitRTX(SFPcontext *ctx, SFPpacket *packet);
static int sfpWriteNoCRC(SFPcontext *ctx, uint8_t octet, size_t *outlen);
static int sfpWrite(SFPcontext *ctx, uint8_t octet, size_t *outlen);

static void sfpBufferOctet(SFPcontext *ctx, uint8_t octet);
static void sfpHandleNAK(SFPcontext *ctx);
static int sfpHandleUSR(SFPcontext *ctx);
static void sfpHandleSYN(SFPcontext *ctx);
static void sfpHandleSYN0(SFPcontext *ctx);
static void sfpHandleSYN1(SFPcontext *ctx);
static void sfpHandleSYN2(SFPcontext *ctx);
static void sfpTransmitHistoryFromSeq(SFPcontext *ctx, SFPseq seq);
static void sfpTransmitHistory(SFPcontext *ctx);
static void sfpTransmitNAK(SFPcontext *ctx, SFPseq seq);
static void sfpResetReceiver(SFPcontext *ctx);
static int sfpHandleFrame(SFPcontext *ctx);
static int sfpCopyOutPacket(SFPcontext *ctx, uint8_t *buf, size_t len, size_t *outlen);

//////////////////////////////////////////////////////////////////////////////

size_t
sfpGetSizeof(void)
{
    return sizeof(SFPcontext);
}

void
sfpInit(SFPcontext *ctx)
{
    ctx->connectState = SFP_CONNECT_STATE_DISCONNECTED;

    ////////////////////////////////////////////////////////////////////////////

    ctx->rx.seq = SFP_INITIAL_SEQ;

    sfpResetReceiver(ctx);

    sfpSetDeliverCallback(ctx, NULL, NULL);

    ////////////////////////////////////////////////////////////////////////////

    ctx->tx.seq = SFP_INITIAL_SEQ;
    ctx->tx.crc = SFP_CRC_PRESET;

    ctx->tx.writebufn = 0;

    potRingbufferInit(&(ctx->tx.history));

    sfpSetWriteCallback(ctx, NULL, NULL);
}

/* Should ideally be called only when sfpDeliverOctet() is not executing. */
void
sfpConnect(SFPcontext *ctx)
{
    /* Very similar to the sfpHandleSYN* functions. All connect states do the same thing. */

    sfpResetReceiver(ctx);
    ctx->rx.seq = SFP_INITIAL_SEQ;

    ctx->tx.seq = SFP_INITIAL_SEQ;
    sfpClearHistory(ctx);
    sfpTransmitSYN0(ctx);
    ctx->connectState = SFP_CONNECT_STATE_SENT_SYN0;
}

int
sfpIsConnected(SFPcontext *ctx)
{
    return SFP_CONNECT_STATE_CONNECTED == ctx->connectState;
}

void
sfpSetDeliverCallback(SFPcontext *ctx, SFPdeliverfun cbfun, void *userdata)
{
    ctx->rx.deliver = cbfun;
    ctx->rx.deliverData = userdata;
}

void
sfpSetWriteCallback(SFPcontext *ctx, SFPwritefun cbfun, void *userdata)
{
    ctx->tx.write = cbfun;
    ctx->tx.writeData = userdata;
}

/* Entry point for receiver. Returns -1 on error, 0 on no error, no buf
 * modification, and > 0 if a packet was written into buf. If buf is NULL, no
 * data will be written to buf, and you must rely on the deliver callback. */
int
sfpDeliverOctet(SFPcontext *ctx, uint8_t octet, uint8_t *buf, size_t len, size_t *outlen)
{
    int ret = 0;

    if (SFP_FLAG == octet) {
        if (SFP_FRAME_STATE_RECEIVING == ctx->rx.frameState) {
            ret = sfpHandleFrame(ctx);
            if (ret && buf) {
                ret = sfpCopyOutPacket(ctx, buf, len, outlen);
            }
        }
        /* If we receive a FLAG while in FRAME_STATE_NEW, this means we have
         * received back-to-back FLAG octets. This is a heartbeat/keepalive, and we
         * simply ignore them. */
        sfpResetReceiver(ctx);
    } else if (SFP_ESC == octet) {
        ctx->rx.escapeState = SFP_ESCAPE_STATE_ESCAPING;
    } else {
        /* All other, non-control octets. */

        if (SFP_ESCAPE_STATE_ESCAPING == ctx->rx.escapeState) {
            octet ^= SFP_ESC_FLIP_BIT;
            ctx->rx.escapeState = SFP_ESCAPE_STATE_NORMAL;
        }

        ctx->rx.crc = _crc_ccitt_update(ctx->rx.crc, octet);

        if (SFP_FRAME_STATE_NEW == ctx->rx.frameState) {
            /* We are receiving the header. */
            ctx->rx.header = octet;
            ctx->rx.frameState = SFP_FRAME_STATE_RECEIVING;
        } else {
            /* We are receiving the payload. */
            sfpBufferOctet(ctx, octet);
        }
    }

    return ret;
}

/* Entry point for transmitter. */
int
sfpWritePacket(SFPcontext *ctx, const uint8_t *buf, size_t len, size_t *outlen)
{
    /* TODO maybe get rid of SFPpacket altogether? Would be more annoying to
     * pass buf and len through everything, but would speed this up for sure. */
    SFPpacket packet;
    memcpy(packet.buf, buf, len);
    packet.len = len;

    int ret = sfpTransmitUSR(ctx, &packet, outlen);

    return ret;
}

//////////////////////////////////////////////////////////////////////////////

static int
isReservedOctet(uint8_t octet)
{
    switch (octet) {
    case SFP_ESC:
    /* fall-through */
    case SFP_FLAG:
        return 1;
    default:
        return 0;
    }
}

static SFPseq
nextSeq(SFPseq seq)
{
    return (seq + 1) & (SFP_SEQ_RANGE - 1);
}

static SFPframetype
getFrameType(SFPheader header)
{

    return (SFPframetype)((header >> SFP_FIRST_CONTROL_BIT) & ((1 << SFP_NUM_CONTROL_BITS) - 1));
}

static SFPseq
getFrameSeq(SFPheader header)
{
    return (header >> SFP_FIRST_SEQ_BIT) & ((1 << SFP_NUM_SEQ_BITS) - 1);
}

//////////////////////////////////////////////////////////////////////////////

static void
sfpResetReceiver(SFPcontext *ctx)
{
    ctx->rx.crc = SFP_CRC_PRESET;
    ctx->rx.escapeState = SFP_ESCAPE_STATE_NORMAL;
    ctx->rx.frameState = SFP_FRAME_STATE_NEW;
    ctx->rx.packet.len = 0;
}

static int
sfpHandleFrame(SFPcontext *ctx)
{
    /* Verify the length. */
    if (SFP_CRC_SIZE > ctx->rx.packet.len) {
        sfpTransmitNAK(ctx, ctx->rx.seq);
        return 0;
    }

    /* Now that the length is verified, we can rewind over the CRC. */
    ctx->rx.packet.len -= SFP_CRC_SIZE;

    /* Verify the CRC. */
    if (SFP_CRC_GOOD != ctx->rx.crc) {
        sfpTransmitNAK(ctx, ctx->rx.seq);
        return 0;
    }

    int ret = 0;

    /* And finally, handle the frame if it all checks out. */
    SFPframetype type = getFrameType(ctx->rx.header);

    switch (type) {
    case SFP_FRAME_USR:
    /* fall-through */
    case SFP_FRAME_RTX:
        ret = sfpHandleUSR(ctx);
        break;
    case SFP_FRAME_NAK: {
        sfpHandleNAK(ctx);
        break;
    }
    case SFP_FRAME_SYN: {
        sfpHandleSYN(ctx);
        break;
    }
    default:
        /* FIXME bitch to the user? */
        /* error: unknown frame type */
        break;
    }

    return ret;
}

/* Return -1 on failure, 1 on success. */
static int
sfpCopyOutPacket(SFPcontext *ctx, uint8_t *buf, size_t len, size_t *outlen)
{
    if (len < ctx->rx.packet.len) {
        return -1;
    } else {
        memcpy(buf, ctx->rx.packet.buf, ctx->rx.packet.len);
        *outlen = ctx->rx.packet.len;
        return 1;
    }
}

/* Handle user frame. */
static int
sfpHandleUSR(SFPcontext *ctx)
{
    {
        switch (ctx->connectState) {
        case SFP_CONNECT_STATE_DISCONNECTED:
            sfpTransmitDIS(ctx);
            return 0;
        case SFP_CONNECT_STATE_SENT_SYN0:
            sfpTransmitSYN0(ctx);
            return 0;
        case SFP_CONNECT_STATE_SENT_SYN1:
            sfpTransmitSYN1(ctx);
            return 0;
        case SFP_CONNECT_STATE_CONNECTED:
        /* fall-through */
        default:
            break;
        }
    }

    int ret = 0;

    SFPseq seq = getFrameSeq(ctx->rx.header);

    if (seq != ctx->rx.seq) {
        SFPframetype type = getFrameType(ctx->rx.header);

        if (SFP_FRAME_USR == type) {
            sfpTransmitNAK(ctx, ctx->rx.seq);
        } else {
        }
    } else {
        /* Good user frame received and accepted--deliver it. */
        if (ctx->rx.deliver) {
            ctx->rx.deliver(ctx->rx.packet.buf, ctx->rx.packet.len, ctx->rx.deliverData);
        }
        ctx->rx.seq = nextSeq(ctx->rx.seq);
        ret = 1;
    }

    return ret;
}

static void
sfpHandleSYN0(SFPcontext *ctx)
{
    /* All connect states do the same thing. */

    sfpResetReceiver(ctx);
    ctx->rx.seq = SFP_INITIAL_SEQ;
    ctx->tx.seq = SFP_INITIAL_SEQ;
    sfpClearHistory(ctx);
    sfpTransmitSYN1(ctx);
    ctx->connectState = SFP_CONNECT_STATE_SENT_SYN1;
}

static void
sfpHandleSYN1(SFPcontext *ctx)
{
    if (SFP_CONNECT_STATE_DISCONNECTED == ctx->connectState) {
        sfpTransmitDIS(ctx);
    } else {
        sfpTransmitSYN2(ctx);
        if (SFP_INITIAL_SEQ != ctx->tx.seq) {
            sfpTransmitHistoryFromSeq(ctx, SFP_INITIAL_SEQ);
        }
        ctx->connectState = SFP_CONNECT_STATE_CONNECTED;
    }
}

static void
sfpHandleSYN2(SFPcontext *ctx)
{
    if (SFP_CONNECT_STATE_DISCONNECTED == ctx->connectState) {
        sfpTransmitDIS(ctx);
    } else if (SFP_CONNECT_STATE_SENT_SYN0 == ctx->connectState) {
        sfpTransmitSYN0(ctx);
    } else {
        if (SFP_INITIAL_SEQ != ctx->tx.seq) {
            sfpTransmitHistoryFromSeq(ctx, SFP_INITIAL_SEQ);
        }
        ctx->connectState = SFP_CONNECT_STATE_CONNECTED;
    }
}

static void
sfpHandleNAK(SFPcontext *ctx)
{
    switch (ctx->connectState) {
    case SFP_CONNECT_STATE_DISCONNECTED:
        sfpTransmitDIS(ctx);
        return;
    case SFP_CONNECT_STATE_SENT_SYN0:
        sfpTransmitSYN0(ctx);
        return;
    case SFP_CONNECT_STATE_SENT_SYN1:
        sfpTransmitSYN1(ctx);
        return;
    case SFP_CONNECT_STATE_CONNECTED:
    /* fall-through */
    default:
        break;
    }

    SFPseq seq = getFrameSeq(ctx->rx.header);

    if (seq != ctx->tx.seq) {
        sfpTransmitHistoryFromSeq(ctx, seq);
    }
}

static void
sfpHandleSYN(SFPcontext *ctx)
{
    SFPseq seq = getFrameSeq(ctx->rx.header);

    switch (seq) {
    case SFP_SEQ_SYN0:
        sfpHandleSYN0(ctx);
        break;
    case SFP_SEQ_SYN1:
        sfpHandleSYN1(ctx);
        break;
    case SFP_SEQ_SYN2:
        sfpHandleSYN2(ctx);
        break;
    case SFP_SEQ_SYN_DIS:
        /* FIXME bitch to the user? */
        ctx->connectState = SFP_CONNECT_STATE_DISCONNECTED;
        break;
    default:
        /* error: SYN with unknown SEQ */
        break;
    }
}

static void
sfpTransmitHistoryFromSeq(SFPcontext *ctx, SFPseq seq)
{
    /* The number of frames we'll have to drop from our history ring buffer in
     * order to fast-forward to the given sequence number. */
    unsigned fastforward = seq - (ctx->tx.seq - potRingbufferSize(&(ctx->tx.history)));

    fastforward &= (SFP_SEQ_RANGE - 1);

    if (potRingbufferSize(&(ctx->tx.history)) > fastforward) {
        unsigned i;
        for (i = 0; i < fastforward; ++i) {
            potRingbufferPopFront(&(ctx->tx.history));
        }
    } else {

        /* Even if we lost frames, the show still has to go on. Resynchronize, and
         * send what frames we have available in our history. */
    }

    /* Synchronize our remote sequence number with the given SEQ. */
    ctx->tx.seq = seq;

    sfpTransmitHistory(ctx);
}

static void
sfpTransmitHistory(SFPcontext *ctx)
{
    size_t reTxCount = potRingbufferSize(&(ctx->tx.history));

    size_t i;
    for (i = 0; i < reTxCount; ++i) {
        sfpTransmitRTX(ctx, potRingbufferAt(&(ctx->tx.history), i));
    }
}

static void
sfpClearHistory(SFPcontext *ctx)
{
    while (!potRingbufferEmpty(&(ctx->tx.history))) {
        potRingbufferPopFront(&(ctx->tx.history));
    }
}

//////////////////////////////////////////////////////////////////////////////

static void
sfpBufferOctet(SFPcontext *ctx, uint8_t octet)
{
    if (SFP_CONFIG_MAX_PACKET_SIZE <= ctx->rx.packet.len) {

        /* Until I have a better idea, just going to pretend we didn't receive
         * anything at all, and just go on with life. If this was caused by a
         * corrupt FLAG octet, then our forthcoming NAK should resynchronize
         * everything. TODO report the error */
        sfpResetReceiver(ctx);
    } else {
        /* Finally, the magic happens. */
        ctx->rx.packet.buf[ctx->rx.packet.len++] = octet;
    }
}

/* Wrapper around sfpBufferedWrite, updating the rolling CRC and escaping
 * reserved octets as necessary. */
static int
sfpWrite(SFPcontext *ctx, uint8_t octet, size_t *outlen)
{
    ctx->tx.crc = _crc_ccitt_update(ctx->tx.crc, octet);
    return sfpWriteNoCRC(ctx, octet, outlen);
}

static int
sfpWriteNoCRC(SFPcontext *ctx, uint8_t octet, size_t *outlen)
{
    size_t n;
    if (outlen) {
        *outlen = 0;
    }

    if (isReservedOctet(octet)) {
        octet ^= SFP_ESC_FLIP_BIT;
        sfpBufferedWrite(SFP_ESC, &n, ctx);
        if (outlen) {
            *outlen += n;
        }
    }
    sfpBufferedWrite(octet, &n, ctx);
    if (outlen) {
        *outlen += n;
    }

    /* FIXME collect return values from sfpBufferedWrite */
    return 0;
}

static void
sfpTransmitNAK(SFPcontext *ctx, SFPseq seq)
{
    SFPheader header = seq << SFP_FIRST_SEQ_BIT;
    header |= SFP_FRAME_NAK << SFP_FIRST_CONTROL_BIT;

    sfpTransmitFrameWithHeader(ctx, header, NULL, NULL);
}

static void
sfpTransmitDIS(SFPcontext *ctx)
{
    SFPheader header = SFP_SEQ_SYN_DIS << SFP_FIRST_SEQ_BIT;
    header |= SFP_FRAME_SYN << SFP_FIRST_CONTROL_BIT;

    sfpTransmitFrameWithHeader(ctx, header, NULL, NULL);
}

static void
sfpTransmitSYN0(SFPcontext *ctx)
{
    SFPheader header = SFP_SEQ_SYN0 << SFP_FIRST_SEQ_BIT;
    header |= SFP_FRAME_SYN << SFP_FIRST_CONTROL_BIT;

    sfpTransmitFrameWithHeader(ctx, header, NULL, NULL);
}

static void
sfpTransmitSYN1(SFPcontext *ctx)
{
    SFPheader header = SFP_SEQ_SYN1 << SFP_FIRST_SEQ_BIT;
    header |= SFP_FRAME_SYN << SFP_FIRST_CONTROL_BIT;

    sfpTransmitFrameWithHeader(ctx, header, NULL, NULL);
}

static void
sfpTransmitSYN2(SFPcontext *ctx)
{
    SFPheader header = SFP_SEQ_SYN2 << SFP_FIRST_SEQ_BIT;
    header |= SFP_FRAME_SYN << SFP_FIRST_CONTROL_BIT;

    sfpTransmitFrameWithHeader(ctx, header, NULL, NULL);
}

static int
sfpTransmitUSR(SFPcontext *ctx, SFPpacket *packet, size_t *outlen)
{
    return sfpTransmitFrameImpl(ctx, packet, outlen, 0);
}

static void
sfpTransmitRTX(SFPcontext *ctx, SFPpacket *packet)
{
    sfpTransmitFrameImpl(ctx, packet, NULL, 1);
}

static int
sfpTransmitFrameImpl(SFPcontext *ctx, SFPpacket *packet, size_t *outlen, int retransmit)
{
    SFPheader header = ctx->tx.seq << SFP_FIRST_SEQ_BIT;

    if (retransmit) {
        header |= SFP_FRAME_RTX << SFP_FIRST_CONTROL_BIT;
        /* Retransmissions come from the history, so we don't put them back in. */
    } else {
        header |= SFP_FRAME_USR << SFP_FIRST_CONTROL_BIT;
        potRingbufferPushBack(&(ctx->tx.history), packet);
    }

    int ret = sfpTransmitFrameWithHeader(ctx, header, packet, outlen);
    ctx->tx.seq = nextSeq(ctx->tx.seq);

    return ret;
}

/* Provided separately from sfpTransmitFrame so that the receiver can
 * use it to send control frames. */
static int
sfpTransmitFrameWithHeader(SFPcontext *ctx, SFPheader header, SFPpacket *packet, size_t *outlen)
{
    size_t n;
    size_t unused_variable = 0; // just so we don't have to write if (outlen) { ... }
                                // every five seconds

    if (!outlen) {
        outlen = &unused_variable;
    }

    *outlen = 0;

    ctx->tx.crc = SFP_CRC_PRESET;

    /* Begin frame. */
    sfpBufferedWrite(SFP_FLAG, &n, ctx);

    *outlen += n;

    sfpWrite(ctx, header, &n);
    *outlen += n;

    if (packet) {
        size_t i;
        for (i = 0; i < packet->len; ++i) {
            sfpWrite(ctx, packet->buf[i], &n);
            *outlen += n;
        }
    }

    /* Send the complement of the CRC, similar to how PPP, HDLC do it. */
    SFPcrc crc = ~ctx->tx.crc;

    size_t i;
    for (i = 0; i < sizeof(crc); ++i) {
        /* At first glance, this might seem bizarre. The "NoCRC" bit simply means
         * that the transmitter's rolling CRC will not be updated by the octet we
         * pass. We don't need to CRC the CRC itself. We write the CRC least
         * significant octet first, so that it is checked correctly on the other
         * end. */
        sfpWriteNoCRC(ctx, crc & 0x00ff, &n);
        *outlen += n;
        crc >>= 8;
    }

    /* End frame. */
    sfpBufferedWrite(SFP_FLAG, &n, ctx);
    *outlen += n;

    sfpFlushWriteBuffer(ctx);

    /* FIXME pass through the return values from sfpWrite* */
    return 0;
}

static void
sfpFlushWriteBuffer(SFPcontext *ctx)
{
    size_t outlen;
    ctx->tx.write(ctx->tx.writebuf, ctx->tx.writebufn, &outlen, ctx->tx.writeData);
    ctx->tx.writebufn = 0;
}

static int
sfpBufferedWrite(uint8_t octet, size_t *outlen, void *data)
{
    SFPcontext *ctx = (SFPcontext *)data;

    if (ctx->tx.writebufn >= SFP_CONFIG_WRITEBUF_SIZE) {
        sfpFlushWriteBuffer(ctx);
    }

    ctx->tx.writebuf[ctx->tx.writebufn++] = octet;
    if (outlen) {
        *outlen = 1;
    }

    return 0;
}
