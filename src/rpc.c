// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c et
/*-----------------------------------------------------------------------------*/
/** @file    rpc.c                                                             */
/** @brief   The Remote Procedure Call system for the robot                    */
/*-----------------------------------------------------------------------------*/

#include "rpc.h"
#include "cassette.h"
#include "portable_endian.h"

#include <stdlib.h>
#include <string.h>

static void rpcPublish(rpc_t *rpc, rpcSubscription_t *sub);
static void rpcPublishClock(rpc_t *rpc, rpcSubscription_t *sub);
static void rpcPublishMotor(rpc_t *rpc, rpcSubscription_t *sub);
static void rpcPublishAll(rpc_t *rpc, rpcSubscription_t *sub);
static void rpcRecvPing(rpc_t *rpc, const message_ping_t *ping);
static void rpcRecvInfo(rpc_t *rpc, const message_info_t *info);
static void rpcRecvInfoNetwork(rpc_t *rpc, const message_info_t *info);
static void rpcRecvRead(rpc_t *rpc, const message_read_t *read);
static void rpcRecvReadPubsub(rpc_t *rpc, const message_read_t *read);
static void rpcRecvReadClock(rpc_t *rpc, const message_read_t *read);
static void rpcRecvReadMotor(rpc_t *rpc, const message_read_t *read);
static void rpcRecvReadCassette(rpc_t *rpc, const message_read_t *read);
static void rpcRecvWrite(rpc_t *rpc, const message_write_t *write);
static void rpcRecvWriteMotor(rpc_t *rpc, const message_write_t *write);
static void rpcRecvWriteCassette(rpc_t *rpc, const message_write_t *write);
static void rpcRecvSubscribe(rpc_t *rpc, const message_subscribe_t *subscribe);
static void rpcRecvUnsubscribe(rpc_t *rpc, const message_unsubscribe_t *unsubscribe);
static int rpcSendData(rpc_t *rpc, uint16_t req_id, uint8_t topic, uint8_t subtopic, uint8_t flag, uint8_t len, uint8_t *value);
static int rpcSendPub(rpc_t *rpc, rpcSubscription_t *sub, uint8_t len, uint8_t *value);
static int rpcSendPubError(rpc_t *rpc, rpcSubscription_t *sub, uint8_t error);
static int rpcSendRep(rpc_t *rpc, const message_read_t *read, uint8_t len, uint8_t *value);
static int rpcSendRepError(rpc_t *rpc, uint16_t req_id, uint8_t topic, uint8_t subtopic, uint8_t error);
static int rpcSubFind(rpc_t *rpc, uint16_t req_id, rpcSubscription_t **subp);
static int rpcSubFree(rpc_t *rpc, rpcSubscription_t **subp);
static void rpcSubReset(rpcSubscription_t *sub);

void
rpcLoop(rpc_t *rpc)
{
    int i;
    uint32_t value32;
    uint16_t value16;
    uint8_t *tbuf = (void *)rpc->tmp;
    uint8_t tlen = 0;
    if (chTimeElapsedSince(rpc->published) >= RPC_PUB_TIMEOUT) {
        for (i = 0; i < RPC_SUB_MAX; i++) {
            if (rpc->subs[i].active) {
                (void)rpcPublish(rpc, &rpc->subs[i]);
            }
        }
        rpc->published = chTimeNow();
    }
    if (chTimeElapsedSince(rpc->sendstats) >= RPC_INFO_TIMEOUT) {
        value32 = (uint32_t)chTimeNow();
        value32 = (uint32_t)(htonl(value32));
        (void)memcpy(tbuf, &value32, 4);
        tbuf += 4;
        tlen += 4;
        value16 = vexSpiGetMainBattery();
        value16 = (uint16_t)(htons(value16));
        (void)memcpy(tbuf, &value16, 2);
        tbuf += 2;
        tlen += 2;
        value16 = vexSpiGetBackupBattery();
        value16 = (uint16_t)(htons(value16));
        (void)memcpy(tbuf, &value16, 2);
        tbuf += 2;
        tlen += 2;
        (void)message_info_frame(&rpc->out.msg.info, MESSAGES_TOPIC_ROBOT, MESSAGES_TOPIC_ROBOT_SUBTOPIC_SPI, tlen,
                                 (void *)rpc->tmp);
        (void)rpcSend(rpc, &rpc->out.msg);
        rpc->sendstats = chTimeNow();
    }
    return;
}

static void
rpcPublish(rpc_t *rpc, rpcSubscription_t *sub)
{
    if (!sub->active) {
        return;
    }
    switch (sub->topic) {
    case MESSAGES_TOPIC_CLOCK:
        (void)rpcPublishClock(rpc, sub);
        break;
    case MESSAGES_TOPIC_MOTOR:
        (void)rpcPublishMotor(rpc, sub);
        break;
    case MESSAGES_TOPIC_ALL:
        (void)rpcPublishAll(rpc, sub);
        break;
    default:
        (void)rpcSendPubError(rpc, sub, MESSAGES_ERROR_BAD_TOPIC);
        (void)rpcSubReset(sub);
        break;
    }
    return;
}

static void
rpcPublishClock(rpc_t *rpc, rpcSubscription_t *sub)
{
    uint64_t value;
    switch (sub->subtopic) {
    case MESSAGES_TOPIC_CLOCK_SUBTOPIC_NOW:
        value = (uint64_t)chTimeNow();
        value = (uint64_t)(htonll(value));
        (void)rpcSendPub(rpc, sub, 8, (void *)&value);
        break;
    default:
        (void)rpcSendPubError(rpc, sub, MESSAGES_ERROR_BAD_SUBTOPIC);
        (void)rpcSubReset(sub);
        break;
    }
    return;
}

static void
rpcPublishMotor(rpc_t *rpc, rpcSubscription_t *sub)
{
    int16_t i;
    int8_t index;
    int8_t value;
    uint8_t *tbuf = (void *)rpc->tmp;
    uint8_t tlen = 0;
    if (sub->subtopic < kVexMotorNum) {
        index = sub->subtopic;
        i = (int16_t)sub->subtopic;
        value = (int8_t)vexMotorGet(i);
        if (value == rpc->motor[index]) {
            return;
        }
        rpc->motor[index] = value;
        (void)rpcSendPub(rpc, sub, 1, (void *)&value);
        return;
    }
    switch (sub->subtopic) {
    case MESSAGES_TOPIC_MOTOR_SUBTOPIC_ALL:
        for (i = kVexMotor_1; i < kVexMotorNum; i++) {
            index = (int8_t)i;
            value = (int8_t)vexMotorGet(i);
            if (value == rpc->motor[index]) {
                continue;
            }
            rpc->motor[index] = value;
            (void)memcpy(tbuf, &index, 1);
            tbuf += 1;
            tlen += 1;
            (void)memcpy(tbuf, &value, 1);
            tbuf += 1;
            tlen += 1;
        }
        if (tlen > 0) {
            (void)rpcSendPub(rpc, sub, tlen, (void *)rpc->tmp);
        }
        break;
    default:
        (void)rpcSendPubError(rpc, sub, MESSAGES_ERROR_BAD_SUBTOPIC);
        (void)rpcSubReset(sub);
        break;
    }
    return;
}

static void
rpcPublishAll(rpc_t *rpc, rpcSubscription_t *sub)
{
    uint8_t topic;
    uint8_t subtopic;
    switch (sub->subtopic) {
    case MESSAGES_TOPIC_ALL_SUBTOPIC_ALL:
        topic = sub->topic;
        subtopic = sub->subtopic;
        sub->topic = MESSAGES_TOPIC_CLOCK;
        sub->subtopic = MESSAGES_TOPIC_CLOCK_SUBTOPIC_NOW;
        (void)rpcPublishClock(rpc, sub);
        sub->topic = MESSAGES_TOPIC_MOTOR;
        sub->subtopic = MESSAGES_TOPIC_MOTOR_SUBTOPIC_ALL;
        (void)rpcPublishMotor(rpc, sub);
        sub->topic = topic;
        sub->subtopic = subtopic;
        break;
    default:
        (void)rpcSendPubError(rpc, sub, MESSAGES_ERROR_BAD_SUBTOPIC);
        (void)rpcSubReset(sub);
        break;
    }
    return;
}

void
rpcRecv(rpc_t *rpc, const message_any_t *message)
{
    int updateHeartbeat = 1;
    switch (message->message.op) {
    case MESSAGES_OP_PING:
        (void)rpcRecvPing(rpc, &message->ping);
        break;
    case MESSAGES_OP_PONG:
        break;
    case MESSAGES_OP_INFO:
        (void)rpcRecvInfo(rpc, &message->info);
        break;
    case MESSAGES_OP_DATA:
        break;
    case MESSAGES_OP_READ:
        (void)rpcRecvRead(rpc, &message->read);
        break;
    case MESSAGES_OP_WRITE:
        (void)rpcRecvWrite(rpc, &message->write);
        break;
    case MESSAGES_OP_SUBSCRIBE:
        (void)rpcRecvSubscribe(rpc, &message->subscribe);
        break;
    case MESSAGES_OP_UNSUBSCRIBE:
        (void)rpcRecvUnsubscribe(rpc, &message->unsubscribe);
        break;
    default:
        updateHeartbeat = 0;
        break;
    }
    if (updateHeartbeat) {
        rpc->heartbeat = chTimeNow();
    }
    return;
}

static void
rpcRecvPing(rpc_t *rpc, const message_ping_t *ping)
{
    rpc->seq_id = ping->seq_id;
    // vex_printf("PING: seq_id=%d\r\n", ping->seq_id);
    (void)message_pong_frame(&rpc->out.msg.pong, ping->seq_id);
    (void)rpcSend(rpc, &rpc->out.msg);
    // vex_printf("PONG: seq_id=%d\r\n", rpc->out.msg.pong.seq_id);
    return;
}

static void
rpcRecvInfo(rpc_t *rpc, const message_info_t *info)
{
    switch (info->topic) {
    case MESSAGES_TOPIC_NETWORK:
        (void)rpcRecvInfoNetwork(rpc, info);
        break;
    default:
        break;
    }
    return;
}

static void
rpcRecvInfoNetwork(rpc_t *rpc, const message_info_t *info)
{
    switch (info->subtopic) {
    case MESSAGES_TOPIC_NETWORK_SUBTOPIC_IPV4:
        if (info->len != 4) {
            return;
        }
        (void)memcpy(rpc->ipv4, info->value, 4);
        break;
    default:
        break;
    }
    return;
}

static void
rpcRecvRead(rpc_t *rpc, const message_read_t *read)
{
    switch (read->topic) {
    case MESSAGES_TOPIC_PUBSUB:
        (void)rpcRecvReadPubsub(rpc, read);
        break;
    case MESSAGES_TOPIC_CLOCK:
        (void)rpcRecvReadClock(rpc, read);
        break;
    case MESSAGES_TOPIC_MOTOR:
        (void)rpcRecvReadMotor(rpc, read);
        break;
    case MESSAGES_TOPIC_CASSETTE:
        (void)rpcRecvReadCassette(rpc, read);
        break;
    default:
        (void)rpcSendRepError(rpc, read->req_id, read->topic, read->subtopic, MESSAGES_ERROR_BAD_TOPIC);
        break;
    }
    return;
}

static void
rpcRecvReadPubsub(rpc_t *rpc, const message_read_t *read)
{
    int i;
    uint8_t flag;
    uint8_t value;
    uint16_t req_id;
    uint8_t *tbuf = (void *)rpc->tmp;
    uint8_t tlen = 0;
    if (read->subtopic < RPC_SUB_MAX) {
        i = (int)read->subtopic;
        if (!rpc->subs[i].active) {
            (void)rpcSendRep(rpc, read, 0, NULL);
            return;
        }
        req_id = rpc->subs[i].req_id;
        req_id = (uint16_t)(htons(req_id));
        (void)memcpy(tbuf, &req_id, 2);
        (void)memcpy(tbuf + 2, &rpc->subs[i].topic, 1);
        (void)memcpy(tbuf + 3, &rpc->subs[i].subtopic, 1);
        (void)rpcSendRep(rpc, read, 4, (void *)tbuf);
        return;
    }
    switch (read->subtopic) {
    case MESSAGES_TOPIC_PUBSUB_SUBTOPIC_COUNT:
        value = (uint8_t)rpcSubFree(rpc, NULL);
        value = RPC_SUB_MAX - value;
        (void)rpcSendRep(rpc, read, 1, (void *)&value);
        break;
    case MESSAGES_TOPIC_PUBSUB_SUBTOPIC_FREE:
        value = (uint8_t)rpcSubFree(rpc, NULL);
        (void)rpcSendRep(rpc, read, 1, (void *)&value);
        break;
    case MESSAGES_TOPIC_PUBSUB_SUBTOPIC_MAX:
        value = RPC_SUB_MAX;
        (void)rpcSendRep(rpc, read, 1, (void *)&value);
        break;
    case MESSAGES_TOPIC_PUBSUB_SUBTOPIC_LIST:
        for (i = 0; i < RPC_SUB_MAX; i++) {
            if (rpc->subs[i].active) {
                (void)memcpy(tbuf, &i, 1);
                tbuf += 1;
                tlen += 1;
                req_id = rpc->subs[i].req_id;
                req_id = (uint16_t)(htons(req_id));
                (void)memcpy(tbuf, &req_id, 2);
                tbuf += 2;
                tlen += 2;
                (void)memcpy(tbuf, &rpc->subs[i].topic, 1);
                tbuf += 1;
                tlen += 1;
                (void)memcpy(tbuf, &rpc->subs[i].subtopic, 1);
                tbuf += 1;
                tlen += 1;
            }
        }
        (void)rpcSendRep(rpc, read, tlen, (void *)rpc->tmp);
        break;
    case MESSAGES_TOPIC_PUBSUB_SUBTOPIC_ALL:
        flag = 0;
        // LIST
        for (i = 0; i < RPC_SUB_MAX; i++) {
            if (rpc->subs[i].active) {
                (void)memcpy(tbuf, &i, 1);
                tbuf += 1;
                tlen += 1;
                req_id = rpc->subs[i].req_id;
                req_id = (uint16_t)(htons(req_id));
                (void)memcpy(tbuf, &req_id, 2);
                tbuf += 2;
                tlen += 2;
                (void)memcpy(tbuf, &rpc->subs[i].topic, 1);
                tbuf += 1;
                tlen += 1;
                (void)memcpy(tbuf, &rpc->subs[i].subtopic, 1);
                tbuf += 1;
                tlen += 1;
            }
        }
        (void)rpcSendData(rpc, read->req_id, read->topic, MESSAGES_TOPIC_PUBSUB_SUBTOPIC_LIST, flag, tlen, (void *)rpc->tmp);
        // COUNT
        value = (uint8_t)rpcSubFree(rpc, NULL);
        value = RPC_SUB_MAX - value;
        (void)rpcSendData(rpc, read->req_id, read->topic, MESSAGES_TOPIC_PUBSUB_SUBTOPIC_COUNT, flag, 1, (void *)&value);
        // FREE
        value = (uint8_t)rpcSubFree(rpc, NULL);
        (void)rpcSendData(rpc, read->req_id, read->topic, MESSAGES_TOPIC_PUBSUB_SUBTOPIC_FREE, flag, 1, (void *)&value);
        // MAX
        value = RPC_SUB_MAX;
        (void)rpcSendData(rpc, read->req_id, read->topic, MESSAGES_TOPIC_PUBSUB_SUBTOPIC_MAX, flag, 1, (void *)&value);
        flag |= MESSAGES_DATA_FLAG_END;
        (void)rpcSendData(rpc, read->req_id, read->topic, read->subtopic, flag, 0, NULL);
        break;
    default:
        (void)rpcSendRepError(rpc, read->req_id, read->topic, read->subtopic, MESSAGES_ERROR_BAD_SUBTOPIC);
        break;
    }
    return;
}

static void
rpcRecvReadClock(rpc_t *rpc, const message_read_t *read)
{
    uint64_t value;
    switch (read->subtopic) {
    case MESSAGES_TOPIC_CLOCK_SUBTOPIC_NOW:
        value = (uint64_t)chTimeNow();
        value = (uint64_t)(htonll(value));
        (void)rpcSendRep(rpc, read, 8, (void *)&value);
        break;
    default:
        (void)rpcSendRepError(rpc, read->req_id, read->topic, read->subtopic, MESSAGES_ERROR_BAD_SUBTOPIC);
        break;
    }
    return;
}

static void
rpcRecvReadMotor(rpc_t *rpc, const message_read_t *read)
{
    int16_t i;
    int8_t index;
    int8_t value;
    uint8_t *tbuf = (void *)rpc->tmp;
    uint8_t tlen = 0;
    if (read->subtopic < kVexMotorNum) {
        index = read->subtopic;
        i = (int16_t)read->subtopic;
        value = (int8_t)vexMotorGet(i);
        if (value != rpc->motor[index]) {
            rpc->motor[index] = value;
        }
        (void)rpcSendRep(rpc, read, 1, (void *)&value);
        return;
    }
    switch (read->subtopic) {
    case MESSAGES_TOPIC_MOTOR_SUBTOPIC_ALL:
        for (i = kVexMotor_1; i < kVexMotorNum; i++) {
            index = (int8_t)i;
            value = (int8_t)vexMotorGet(i);
            if (value != rpc->motor[index]) {
                rpc->motor[index] = value;
            }
            (void)memcpy(tbuf, &index, 1);
            tbuf += 1;
            tlen += 1;
            (void)memcpy(tbuf, &value, 1);
            tbuf += 1;
            tlen += 1;
        }
        (void)rpcSendRep(rpc, read, tlen, (void *)rpc->tmp);
        break;
    default:
        (void)rpcSendRepError(rpc, read->req_id, read->topic, read->subtopic, MESSAGES_ERROR_BAD_SUBTOPIC);
        break;
    }
    return;
}

static void
rpcRecvReadCassette(rpc_t *rpc, const message_read_t *read)
{
    uint8_t flag;
    uint8_t value;
    uint32_t rlen = 0;
    uint8_t *tbuf = (void *)rpc->tmp;
    uint8_t tlen = 0;
    user_param *fp = NULL;
    if (read->subtopic < cassetteMax()) {
        if (rpc->fp != NULL) {
            (void)vexFlashUserParamWrite(rpc->fp);
            (void)rpcSendRep(rpc, read, 0, NULL);
            return;
        }
        fp = cassetteOpenRead(read->subtopic);
        if (fp == NULL) {
            (void)rpcSendRep(rpc, read, 0, NULL);
            return;
        }
        flag = 0;
        rlen = (uint32_t)fp->data[0];
        if (rlen > 0 && rlen < 32) {
            (void)memcpy(tbuf, &fp->data[1], rlen);
            tbuf += rlen;
            tlen += rlen;
            tbuf = (void *)rpc->tmp;
            (void)rpcSendData(rpc, read->req_id, read->topic, read->subtopic, flag, tlen, tbuf);
            tlen = 0;
        } else {
            rlen = 0;
        }
        rlen = (uint32_t)(htonl(rlen));
        (void)rpcSendRep(rpc, read, 4, (void *)&rlen);
        return;
    }
    switch (read->subtopic) {
    case MESSAGES_TOPIC_CASSETTE_SUBTOPIC_OPEN:
        if (rpc->fp != NULL) {
            rlen = (uint32_t)fp->data[0];
            if (rlen > 31) {
                rlen = 0;
            }
        }
        value = (uint8_t)rpc->cassette;
        (void)memcpy(tbuf, &value, 1);
        tbuf += 1;
        tlen += 1;
        rlen = (uint32_t)(htonl(rlen));
        (void)memcpy(tbuf, &rlen, 4);
        tbuf += 4;
        tlen += 4;
        tbuf = (void *)rpc->tmp;
        (void)rpcSendRep(rpc, read, tlen, (void *)tbuf);
        break;
    case MESSAGES_TOPIC_CASSETTE_SUBTOPIC_COUNT:
        value = (uint8_t)cassetteCount();
        (void)rpcSendRep(rpc, read, 1, (void *)&value);
        break;
    case MESSAGES_TOPIC_CASSETTE_SUBTOPIC_FREE:
        value = (uint8_t)cassetteFree();
        (void)rpcSendRep(rpc, read, 1, (void *)&value);
        break;
    case MESSAGES_TOPIC_CASSETTE_SUBTOPIC_MAX:
        value = (uint8_t)cassetteMax();
        (void)rpcSendRep(rpc, read, 1, (void *)&value);
        break;
    default:
        (void)rpcSendRepError(rpc, read->req_id, read->topic, read->subtopic, MESSAGES_ERROR_BAD_SUBTOPIC);
        break;
    }
    return;
}

static void
rpcRecvWrite(rpc_t *rpc, const message_write_t *write)
{
    switch (write->topic) {
    case MESSAGES_TOPIC_MOTOR:
        (void)rpcRecvWriteMotor(rpc, write);
        break;
    case MESSAGES_TOPIC_CASSETTE:
        (void)rpcRecvWriteCassette(rpc, write);
        break;
    default:
        (void)rpcSendRepError(rpc, write->req_id, write->topic, write->subtopic, MESSAGES_ERROR_BAD_TOPIC);
        break;
    }
    return;
}

static void
rpcRecvWriteMotor(rpc_t *rpc, const message_write_t *write)
{
    (void)rpc;
    uint8_t n;
    uint8_t i;
    int8_t index;
    int8_t value;
    uint8_t *wbuf = write->value;
    // (void)vex_printf("GOT TO HERE: %d %d\r\n", write->subtopic);
    if (write->subtopic == MESSAGES_TOPIC_MOTOR_SUBTOPIC_ALL) {
        // (void)vex_printf("WRITING ALL?\r\n");
        if (write->len == 0 || (write->len % 2) != 0) {
            // (void)vex_printf("BAD LEN: %d \%2 = %d\r\n", write->len, write->len % 2);
            return;
        }
        n = (write->len / 2);
        for (i = 0; i < n; i++) {
            index = (int8_t)(*wbuf);
            wbuf += 1;
            value = (int8_t)(*wbuf);
            wbuf += 1;
            if (index >= kVexMotor_1 && index < kVexMotorNum) {
                (void)vexMotorSet((int16_t)index, (int16_t)value);
            }
        }
    } else if (write->subtopic >= kVexMotorNum) {
        // (void)vex_printf("BAD SUBTOPIC: %d >= %d\r\n", write->subtopic, kVexMotorNum);
        return;
    } else if (write->len != 1) {
        // (void)vex_printf("BAD WRITE LENGTH: %d\r\n", write->len);
        return;
    } else {
        index = (int8_t)write->subtopic;
        value = (int8_t)(*wbuf);
        // (void)vex_printf("WRITE MOTOR: %d -> %d\r\n", (int16_t)index, (int16_t)value);
        (void)vexMotorSet((int16_t)index, (int16_t)value);
    }
    return;
}

static void
rpcRecvWriteCassette(rpc_t *rpc, const message_write_t *write)
{
    uint8_t *wbuf = write->value;
    uint8_t wlen = 0;
    switch (write->subtopic) {
    case MESSAGES_TOPIC_CASSETTE_SUBTOPIC_OPEN:
        if (write->len != 1) {
            return;
        }
        if (*wbuf >= cassetteMax()) {
            return;
        }
        if (rpc->cassette != 0xff) {
            return;
        }
        rpc->cassette = *wbuf;
        rpc->fp = cassetteOpenWrite(rpc->cassette);
        if (rpc->fp == NULL) {
            rpc->cassette = 0xff;
        }
        return;
    case MESSAGES_TOPIC_CASSETTE_SUBTOPIC_CLOSE:
        if (write->len != 1) {
            return;
        }
        if (*wbuf >= cassetteMax()) {
            return;
        }
        if (rpc->cassette == 0xff && rpc->fp == NULL) {
            return;
        }
        if (rpc->cassette != *wbuf) {
            return;
        }
        rpc->cassette = 0xff;
        (void)vexFlashUserParamWrite(rpc->fp);
        rpc->fp = NULL;
        return;
    case MESSAGES_TOPIC_CASSETTE_SUBTOPIC_WRITE:
        if (write->len == 1 || write->len > 2) {
            return;
        }
        if (*wbuf >= cassetteMax()) {
            return;
        }
        if (rpc->cassette == 0xff && rpc->fp == NULL) {
            return;
        }
        if (rpc->cassette != *wbuf) {
            return;
        }
        wbuf += 1;
        wlen = (uint8_t)rpc->fp->data[0];
        if (wlen >= 31) {
            rpc->cassette = 0xff;
            rpc->fp = NULL;
            return;
        }
        rpc->fp->data[wlen] = (unsigned char)*wbuf;
        wlen += 1;
        rpc->fp->data[0] = (unsigned char)wlen;
        (void)vexFlashUserParamWrite(rpc->fp);
        return;
    default:
        break;
    }
    return;
}

static void
rpcRecvSubscribe(rpc_t *rpc, const message_subscribe_t *subscribe)
{
    rpcSubscription_t tmp = {.active = 1, .req_id = subscribe->req_id, .topic = subscribe->topic, .subtopic = subscribe->subtopic};
    rpcSubscription_t *sub = NULL;
    if (rpcSubFind(rpc, subscribe->req_id, NULL) != 0) {
        sub = &tmp;
        (void)rpcSendPubError(rpc, sub, MESSAGES_ERROR_BAD_REQ_ID);
    }
    if (rpcSubFree(rpc, &sub) == 0) {
        sub = &tmp;
        (void)rpcSendPubError(rpc, sub, MESSAGES_ERROR_SUB_MAX);
        return;
    }
    (void)memcpy(sub, &tmp, sizeof(rpcSubscription_t));
    return;
}

static void
rpcRecvUnsubscribe(rpc_t *rpc, const message_unsubscribe_t *unsubscribe)
{
    uint8_t flag = (MESSAGES_DATA_FLAG_PUB | MESSAGES_DATA_FLAG_END);
    rpcSubscription_t tmp = {
        .active = 1, .req_id = unsubscribe->req_id, .topic = MESSAGES_TOPIC_ALL, .subtopic = MESSAGES_TOPIC_ALL_SUBTOPIC_ALL};
    rpcSubscription_t *sub = NULL;
    if (rpcSubFind(rpc, unsubscribe->req_id, &sub) == 0) {
        sub = &tmp;
        (void)rpcSendPubError(rpc, sub, MESSAGES_ERROR_BAD_REQ_ID);
    }
    (void)rpcSendData(rpc, sub->req_id, sub->topic, sub->subtopic, flag, 0, NULL);
    (void)rpcSubReset(sub);
    return;
}

int
rpcSend(rpc_t *rpc, const message_any_t *message)
{
    if (rpc->writePacket == NULL) {
        return -1;
    }
    int retval;
    size_t outlen;
    retval = message_serialize(message, rpc->out.buf, SFP_CONFIG_MAX_PACKET_SIZE, &outlen);
    if (retval == 0) {
        (void)rpc->writePacket(rpc->out.buf, outlen, NULL, (void *)rpc);
    }
    return retval;
}

static int
rpcSendData(rpc_t *rpc, uint16_t req_id, uint8_t topic, uint8_t subtopic, uint8_t flag, uint8_t len, uint8_t *value)
{
    (void)message_data_frame(&rpc->out.msg.data, req_id, topic, subtopic, flag, (uint32_t)(chTimeElapsedSince(rpc->timestamp)), len,
                             value);
    return rpcSend(rpc, &rpc->out.msg);
}

static int
rpcSendPub(rpc_t *rpc, rpcSubscription_t *sub, uint8_t len, uint8_t *value)
{
    uint8_t flag = (MESSAGES_DATA_FLAG_PUB);
    return rpcSendData(rpc, sub->req_id, sub->topic, sub->subtopic, flag, len, value);
}

static int
rpcSendPubError(rpc_t *rpc, rpcSubscription_t *sub, uint8_t error)
{
    uint8_t flag = (MESSAGES_DATA_FLAG_PUB | MESSAGES_DATA_FLAG_ERROR | MESSAGES_DATA_FLAG_END);
    return rpcSendData(rpc, sub->req_id, sub->topic, sub->subtopic, flag, 1, (void *)&error);
}

static int
rpcSendRep(rpc_t *rpc, const message_read_t *read, uint8_t len, uint8_t *value)
{
    uint8_t flag = (MESSAGES_DATA_FLAG_END);
    return rpcSendData(rpc, read->req_id, read->topic, read->subtopic, flag, len, value);
}

static int
rpcSendRepError(rpc_t *rpc, uint16_t req_id, uint8_t topic, uint8_t subtopic, uint8_t error)
{
    uint8_t flag = (MESSAGES_DATA_FLAG_ERROR | MESSAGES_DATA_FLAG_END);
    return rpcSendData(rpc, req_id, topic, subtopic, flag, 1, (void *)&error);
}

static int
rpcSubFind(rpc_t *rpc, uint16_t req_id, rpcSubscription_t **subp)
{
    int i;
    for (i = 0; i < RPC_SUB_MAX; i++) {
        if (rpc->subs[i].active && rpc->subs[i].req_id == req_id) {
            if (subp != NULL) {
                *subp = &rpc->subs[i];
                return 1;
            }
        }
    }
    if (subp != NULL) {
        *subp = NULL;
    }
    return 0;
}

static int
rpcSubFree(rpc_t *rpc, rpcSubscription_t **subp)
{
    int i;
    int nextCnt = 0;
    rpcSubscription_t *nextSub = NULL;
    for (i = 0; i < RPC_SUB_MAX; i++) {
        if (!rpc->subs[i].active) {
            if (nextSub == NULL) {
                nextSub = &rpc->subs[i];
            }
            nextCnt++;
        }
    }
    if (subp != NULL) {
        *subp = nextSub;
    }
    return nextCnt;
}

static void
rpcSubReset(rpcSubscription_t *sub)
{
    sub->active = 0;
    sub->req_id = 0;
    sub->topic = 0;
    sub->subtopic = 0;
}
