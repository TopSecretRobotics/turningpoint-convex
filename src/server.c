// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c et
/*-----------------------------------------------------------------------------*/
/** @file    server.c                                                          */
/** @brief   The Raspberry Pi 3 server for the robot                           */
/*-----------------------------------------------------------------------------*/

#include "server.h"
#include "rpc.h"

#include <stdlib.h>
#include <string.h>

// types for server
typedef enum serverState_t { serverStateConnected = 0, serverStateDisconnected } serverState_t;

typedef struct server_s {
    rpc_t rpc;
    SerialDriver *sd;
    serverState_t state;
    SFPcontext sfp;
} server_t;

// storage for server
static server_t server;

// working area for server task
static WORKING_AREA(waserver, 1024);

// thread and dead timer
static Thread *serverThreadPointer = NULL;
static long serverThreadDeadTimer = 0;

// private functions
static msg_t serverThread(void *arg);
static void serverReset(server_t *ctx);
static void serverRead(uint8_t *buf, size_t len, void *userdata);
static int serverWrite(uint8_t *octets, size_t len, size_t *outlen, void *userdata);
static int serverWritePacket(const uint8_t *octets, size_t len, size_t *outlen, void *userdata);
static void serverCheckConnection(server_t *ctx);

/*-----------------------------------------------------------------------------*/
/** @brief      Assign UART port to the server.                                */
/** @param[in]  sd The serial driver to use for the server                     */
/*-----------------------------------------------------------------------------*/
void
serverSetup(SerialDriver *sd)
{
    server.sd = sd;
    server.rpc.writePacket = serverWritePacket;
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize the Raspberry Pi 3 server.                          */
/*-----------------------------------------------------------------------------*/
void
serverInit(void)
{
    serverReset(&server);
    SerialConfig serialConfig = {115200, 0, USART_CR2_STOP1_BITS, 0}; // 115200 or 230400
    sdStart(server.sd, &serialConfig);
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start the server system thread                                 */
/*-----------------------------------------------------------------------------*/
void
serverStart(void)
{
    if (chTimeElapsedSince(serverThreadDeadTimer) > 300) {
        serverThreadPointer = NULL;
    }
    if (serverThreadPointer != NULL) {
        return;
    }
    serverThreadPointer = chThdCreateStatic(waserver, sizeof(waserver), NORMALPRIO - 1, serverThread, NULL);
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Checks whether the server is connected or not.                 */
/*-----------------------------------------------------------------------------*/
int
serverIsConnected(void)
{
    return (server.state == serverStateConnected);
}

/*-----------------------------------------------------------------------------*/
/** @brief      Checks whether the server is connected or not.                 */
/*-----------------------------------------------------------------------------*/
serverIpv4_t
serverGetIpv4(void)
{
    serverIpv4_t ipv4;
    (void)memcpy(&ipv4, &server.rpc.ipv4, 4);
    return ipv4;
}

/*-----------------------------------------------------------------------------*/
/** @brief      The server thread                                              */
/** @param[in]  arg Unused                                                     */
/** @return     (msg_t) 0                                                      */
/*-----------------------------------------------------------------------------*/
static msg_t
serverThread(void *arg)
{
    // Unused
    (void)arg;

    // Register the task
    vexTaskRegister("server");

    // local variables
    size_t rlen = 0;
    uint8_t *rbuf = NULL;
    server_t *srv = &server;

    // reset the heartbeat and published timers
    srv->rpc.timestamp = srv->rpc.heartbeat = srv->rpc.published = srv->rpc.sendstats = chTimeNow();

    while (!chThdShouldTerminate()) {
        // vex_printf("0 READ\r\n");
        serverThreadDeadTimer = chTimeNow();
        rlen = sdAsynchronousRead(srv->sd, srv->rpc.in.buf, SFP_CONFIG_MAX_PACKET_SIZE);
        rbuf = srv->rpc.in.buf;
        // vex_printf("1 READ: rlen=%lu\r\n", rlen);
        // vex_printf("0 DELIVER\r\n");
        while (rlen-- > 0) {
            (void)sfpDeliverOctet(&srv->sfp, *rbuf, NULL, 0, NULL);
            rbuf++;
        }
        // vex_printf("1 DELIVER\r\n");
        // vex_printf("0 CHECK\r\n");
        (void)serverCheckConnection(srv);
        // vex_printf("1 CHECK\r\n");
        if (serverIsConnected()) {
            // vex_printf("0 LOOP\r\n");
            (void)rpcLoop(&srv->rpc);
            // vex_printf("1 LOOP\r\n");
        }
        // vex_printf("0 SLEEP\r\n");
        vexSleep(500);
        // vex_printf("1 SLEEP\r\n");
    }

    (void)serverReset(srv);
    serverThreadPointer = NULL;
    serverThreadDeadTimer = 0;

    return ((msg_t)0);
}

static void
serverReset(server_t *srv)
{
    int i;
    serverIpv4_t ipv4Empty = {{0, 0, 0, 0}};
    srv->state = serverStateDisconnected;
    srv->rpc.seq_id = 0;
    srv->rpc.timestamp = chTimeNow();
    for (i = 0; i < RPC_SUB_MAX; i++) {
        srv->rpc.subs[i].active = 0;
        srv->rpc.subs[i].req_id = 0;
        srv->rpc.subs[i].topic = 0;
        srv->rpc.subs[i].subtopic = 0;
    }
    (void)memcpy(&srv->rpc.ipv4, &ipv4Empty, 4);
    (void)sfpInit(&srv->sfp);
    (void)sfpSetDeliverCallback(&srv->sfp, serverRead, (void *)srv);
    (void)sfpSetWriteCallback(&srv->sfp, serverWrite, (void *)srv);
    return;
}

static void
serverRead(uint8_t *buf, size_t len, void *userdata)
{
    server_t *srv = (void *)userdata;
    if (message_deserialize(&srv->rpc.in.msg, buf, len) == 0) {
        (void)rpcRecv(&srv->rpc, &srv->rpc.in.msg);
    }
    (void)serverCheckConnection(srv);
    return;
}

static int
serverWrite(uint8_t *octets, size_t len, size_t *outlen, void *userdata)
{
    server_t *srv = (void *)userdata;
    size_t wlen = 0;
    size_t wcnt = 0;
    wlen = sdAsynchronousWrite(srv->sd, octets, len);
    wcnt += wlen;
    len -= wlen;
    while (len > 0) {
        octets += wlen;
        vexSleep(2);
        wlen = sdAsynchronousWrite(srv->sd, octets, len);
        wcnt += wlen;
        len -= wlen;
    }
    if (outlen != NULL) {
        *outlen = wcnt;
    }
    (void)serverCheckConnection(srv);
    return 0;
}

static int
serverWritePacket(const uint8_t *octets, size_t len, size_t *outlen, void *userdata)
{
    server_t *srv = (void *)userdata;
    return sfpWritePacket(&srv->sfp, octets, len, outlen);
}

static void
serverCheckConnection(server_t *srv)
{
    if (srv->state == serverStateDisconnected) {
        if (sfpIsConnected(&srv->sfp)) {
            int i;
            srv->rpc.timestamp = srv->rpc.heartbeat = srv->rpc.published = srv->rpc.sendstats = chTimeNow();
            for (i = 0; i < RPC_SUB_MAX; i++) {
                srv->rpc.subs[i].active = false;
                srv->rpc.subs[i].req_id = 0;
                srv->rpc.subs[i].topic = 0;
                srv->rpc.subs[i].subtopic = 0;
            }
            srv->rpc.cassette = 0xff;
            srv->rpc.fp = NULL;
            srv->state = serverStateConnected;
        }
    } else if (srv->state == serverStateConnected) {
        if (!sfpIsConnected(&srv->sfp) || (chTimeElapsedSince(srv->rpc.heartbeat) > 5000) ||
            (chTimeElapsedSince(srv->rpc.timestamp) > 2147483647)) {
            (void)serverReset(srv);
        }
    }
}
