
// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*
 * rpc.h
 */

#ifndef RPC_H_

#define RPC_H_

#include "ch.h"  // needs for all ChibiOS programs
#include "hal.h" // hardware abstraction layer header
#include "vex.h" // vex library header

#include "messages.h"
#include "serial_framing_protocol.h"
#include "vexflash.h"

#define RPC_SUB_MAX 10
#define RPC_PUB_TIMEOUT 25
#define RPC_INFO_TIMEOUT 1000

typedef int (*rpcWritePacket_t)(const uint8_t *octets, size_t len, size_t *outlen, void *userdata);

typedef struct rpcBuffer_s {
    uint8_t buf[SFP_CONFIG_MAX_PACKET_SIZE];
    message_any_t msg;
} rpcBuffer_t;

typedef struct rpcSubscription_s {
    bool active;
    uint16_t req_id;
    uint8_t topic;
    uint8_t subtopic;
} rpcSubscription_t;

typedef struct rpc_s {
    uint8_t seq_id;
    uint8_t ipv4[4];
    int8_t motor[10];
    uint8_t cassette;
    user_param *fp;
    uint8_t tmp[SFP_CONFIG_MAX_PACKET_SIZE];
    rpcBuffer_t in;
    rpcBuffer_t out;
    uint32_t timestamp;
    uint32_t heartbeat;
    uint32_t published;
    uint32_t sendstats;
    rpcWritePacket_t writePacket;
    rpcSubscription_t subs[RPC_SUB_MAX];
} rpc_t;

#ifdef __cplusplus
extern "C" {
#endif

extern void rpcLoop(rpc_t *rpc);
extern void rpcRecv(rpc_t *rpc, const message_any_t *message);
extern int rpcSend(rpc_t *rpc, const message_any_t *message);

#ifdef __cplusplus
}
#endif

#endif
