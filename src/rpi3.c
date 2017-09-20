// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c et
/*-----------------------------------------------------------------------------*/
/** @file    rpi3.c                                                            */
/** @brief   The Raspberry Pi 3 system for the robot                           */
/*-----------------------------------------------------------------------------*/

#include "rpi3.h"
#include "serial_framing_protocol.h"
#include "messages.h"
#include "portable_endian.h"

#include <stdlib.h>
#include <string.h>

// types for rpi3
typedef enum rpi3State { rpi3StateSFPdisconnected = 0, rpi3StateSFPconnected } rpi3State;

typedef struct rpi3_s {
    SerialDriver *sd;
    message_any_t rmsg;
    message_any_t wmsg;
    uint8_t rbuf[SFP_CONFIG_MAX_PACKET_SIZE];
    uint8_t wbuf[SFP_CONFIG_MAX_PACKET_SIZE];
    uint32_t lastHeartbeat;
    rpi3State state;
    SFPcontext sfp;
} rpi3_t;

// storage for rpi3
static rpi3_t rpi3;

// working area for rpi3 task
static WORKING_AREA(warpi3, 512);

// thread and dead timer
static Thread *rpi3ThreadPointer = NULL;
static long rpi3ThreadDeadTimer = 0;

// private functions
static msg_t rpi3Thread(void *arg);
static void rpi3Reset(rpi3_t *ctx);
static void rpi3Read(uint8_t *buf, size_t len, void *userdata);
static void rpi3HandleMessage(rpi3_t *ctx, const message_any_t *request);
static void rpi3HandleRead(rpi3_t *ctx, const message_read_t *read);
static void rpi3HandleReadClock(rpi3_t *ctx, const message_read_t *read);
static void rpi3HandleReadMotor(rpi3_t *ctx, const message_read_t *read);
static void rpi3HandleWrite(rpi3_t *ctx, const message_write_t *write);
static void rpi3HandleWriteMotor(rpi3_t *ctx, const message_write_t *write);
static int rpi3SendEmptyError(rpi3_t *ctx, uint16_t req_id);
static int rpi3SendMessage(rpi3_t *ctx, const message_any_t *reply);
static int rpi3Write(uint8_t *octets, size_t len, size_t *outlen, void *userdata);
static void rpi3CheckConnection(rpi3_t *ctx);

/*-----------------------------------------------------------------------------*/
/** @brief      Assign rpi3 display to the rpi3 system.                        */
/** @param[in]  display The display to use for the rpi3 system                 */
/*-----------------------------------------------------------------------------*/
void
rpi3Setup(SerialDriver *sd)
{
    rpi3.sd = sd;
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Initialize the Raspberry Pi 3 system.                          */
/*-----------------------------------------------------------------------------*/
void
rpi3Init(void)
{
    rpi3Reset(&rpi3);
    // 230400
    SerialConfig serialConfig = {115200, 0, USART_CR2_STOP1_BITS, 0};
    // SerialConfig serialConfig = {115200,
    //                              USART_CR1_PS | USART_CR1_PCE | USART_CR1_M, // odd parity
    //                              USART_CR2_STOP1_BITS, 0};
    sdStart(rpi3.sd, &serialConfig);
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Start the rpi3 system thread                                   */
/*-----------------------------------------------------------------------------*/
void
rpi3Start(void)
{
    if ((chTimeNow() - rpi3ThreadDeadTimer) > 300) {
        rpi3ThreadPointer = NULL;
    }
    if (rpi3ThreadPointer != NULL) {
        return;
    }
    rpi3ThreadPointer = chThdCreateStatic(warpi3, sizeof(warpi3), NORMALPRIO - 1, rpi3Thread, NULL);
    return;
}

/*-----------------------------------------------------------------------------*/
/** @brief      Checks whether the rpi3 is connected or not.                   */
/*-----------------------------------------------------------------------------*/
int
rpi3IsConnected(void)
{
    return (rpi3.state == rpi3StateSFPconnected);
}

/*-----------------------------------------------------------------------------*/
/** @brief      The rpi3 system thread                                         */
/** @param[in]  arg Unused                                                     */
/** @return     (msg_t) 0                                                      */
/*-----------------------------------------------------------------------------*/
static msg_t
rpi3Thread(void *arg)
{
    // Unused
    (void)arg;

    // Register the task
    vexTaskRegister("rpi3");

    // number of bytes read
    size_t readLength = 0;

    // pointer to bytes read
    uint8_t *readBuffer = NULL;

    // pointer to context
    rpi3_t *ctx = &rpi3;
    ctx->lastHeartbeat = chTimeNow();

    unsigned long prevTime = chTimeNow();
    unsigned long nextTime = chTimeNow();

    while (!chThdShouldTerminate()) {
        rpi3ThreadDeadTimer = chTimeNow();
        readLength = sdAsynchronousRead(ctx->sd, ctx->rbuf, SFP_CONFIG_MAX_PACKET_SIZE);
        readBuffer = ctx->rbuf;
        while ((readLength--) > 0) {
            sfpDeliverOctet(&ctx->sfp, *readBuffer, NULL, 0, NULL);
            readBuffer++;
        }
        rpi3CheckConnection(ctx);
        // if (rpi3IsConnected()) {
        //     nextTime = chTimeNow();
        //     if ((nextTime - prevTime) > 7000) {
        //         char *info_value = "hello";
        //         (void)message_info_frame(&ctx->wmsg.info, 5, (void *)info_value);
        //         (void)rpi3SendMessage(ctx, &ctx->wmsg);
        //         prevTime = nextTime;
        //     }
        // } else {
        //     prevTime = nextTime = chTimeNow();
        // }
        if (!rpi3IsConnected()) {
            prevTime = nextTime = chTimeNow();
        }
        vexSleep(2);
    }

    (void)rpi3Reset(ctx);

    rpi3ThreadPointer = NULL;
    rpi3ThreadDeadTimer = 0;

    return ((msg_t)0);
}

static void
rpi3Reset(rpi3_t *ctx)
{
    ctx->state = rpi3StateSFPdisconnected;
    sfpInit(&ctx->sfp);
    sfpSetDeliverCallback(&ctx->sfp, rpi3Read, (void *)ctx);
    sfpSetWriteCallback(&ctx->sfp, rpi3Write, (void *)ctx);
    return;
}

static void
rpi3Read(uint8_t *buf, size_t len, void *userdata)
{
    rpi3_t *ctx = (void *)userdata;
    if (message_deserialize(&ctx->rmsg, buf, len) == 0) {
        (void)rpi3HandleMessage(ctx, &ctx->rmsg);
    }
    rpi3CheckConnection(ctx);
    return;
}

static void
rpi3HandleMessage(rpi3_t *ctx, const message_any_t *request)
{
    switch (request->message.op) {
        case MESSAGES_OP_PING:
            ctx->lastHeartbeat = chTimeNow();
            (void)message_pong_frame(&ctx->wmsg.pong, request->ping.seq_id);
            (void)rpi3SendMessage(ctx, &ctx->wmsg);
            break;
        case MESSAGES_OP_PONG:
            break;
        case MESSAGES_OP_INFO:
            break;
        case MESSAGES_OP_DATA:
            break;
        case MESSAGES_OP_READ:
            (void)rpi3HandleRead(ctx, &request->read);
            break;
        case MESSAGES_OP_WRITE:
            (void)rpi3HandleWrite(ctx, &request->write);
            break;
        case MESSAGES_OP_SUBSCRIBE:
            (void)rpi3SendEmptyError(ctx, request->subscribe.req_id);
            break;
        case MESSAGES_OP_UNSUBSCRIBE:
            break;
        case MESSAGES_OP_PUBLISH:
            break;
        default:
            break;
    }
    return;
}

static void
rpi3HandleRead(rpi3_t *ctx, const message_read_t *read)
{
    // vexLcdPrintf(VEX_LCD_DISPLAY_1, VEX_LCD_LINE_2, "%d", read->req_id);
    switch (read->type) {
        case MESSAGES_TYPE_CLOCK:
            (void)rpi3HandleReadClock(ctx, read);
            break;
        case MESSAGES_TYPE_MOTOR:
            (void)rpi3HandleReadMotor(ctx, read);
            break;
        default:
            (void)rpi3SendEmptyError(ctx, read->req_id);
            break;
    }
    return;
}

static void
rpi3HandleReadClock(rpi3_t *ctx, const message_read_t *read)
{
    uint32_t value;
    switch (read->topic) {
        case 0:
            value = chTimeNow();
            value = (uint32_t)(htonl(value));
            (void)message_data_frame(&ctx->wmsg.data, read->req_id, 0, 4, (void *)&value);
            (void)rpi3SendMessage(ctx, &ctx->wmsg);
            break;
        default:
            (void)rpi3SendEmptyError(ctx, read->req_id);
            break;
    }
    return;
}

static void
rpi3HandleReadMotor(rpi3_t *ctx, const message_read_t *read)
{
    int16_t index;
    int16_t value;
    index = (int16_t)read->topic;
    value = vexMotorGet(index);
    value = (int16_t)(htons(value));
    (void)message_data_frame(&ctx->wmsg.data, read->req_id, 0, 2, (void *)&value);
    (void)rpi3SendMessage(ctx, &ctx->wmsg);
    return;
}

static void
rpi3HandleWrite(rpi3_t *ctx, const message_write_t *write)
{
    switch (write->type) {
        case MESSAGES_TYPE_MOTOR:
            (void)rpi3HandleWriteMotor(ctx, write);
            break;
        default:
            // (void)rpi3SendEmptyError(ctx, write->req_id);
            break;
    }
    return;
}

static void
rpi3HandleWriteMotor(rpi3_t *ctx, const message_write_t *write)
{
    (void)ctx;
    if (write->len != 2) {
        return;
    }
    int16_t index;
    int16_t value;
    index = (int16_t)write->topic;
    (void)memcpy(&value, write->value, 2);
    value = (int16_t)(ntohs(value));
    (void)vexMotorSet(index, value);
    return;
}

static int
rpi3SendEmptyError(rpi3_t *ctx, uint16_t req_id)
{
    (void)message_data_frame(&ctx->wmsg.data, req_id, MESSAGES_DATA_FLAG_ERROR, 0, NULL);
    return rpi3SendMessage(ctx, &ctx->wmsg);
}

static int
rpi3SendMessage(rpi3_t *ctx, const message_any_t *reply)
{
    int retval;
    size_t outlen;
    retval = message_serialize(reply, ctx->wbuf, SFP_CONFIG_MAX_PACKET_SIZE, &outlen);
    if (retval == 0) {
        (void)sfpWritePacket(&ctx->sfp, ctx->wbuf, outlen, NULL);
    }
    return retval;
}

static int
rpi3Write(uint8_t *octets, size_t len, size_t *outlen, void *userdata)
{
    rpi3_t *ctx = (void *)userdata;
    size_t writeLength = 0;
    size_t writeTotal = 0;
    writeLength = sdAsynchronousWrite(ctx->sd, octets, len);
    writeTotal += writeLength;
    len -= writeLength;
    while (len > 0) {
        octets += writeLength;
        vexSleep(2);
        writeLength = sdAsynchronousWrite(ctx->sd, octets, len);
        writeTotal += writeLength;
        len -= writeLength;
    }
    if (outlen != NULL) {
        *outlen = writeTotal;
    }
    rpi3CheckConnection(ctx);
    return 0;
}

static void
rpi3CheckConnection(rpi3_t *ctx)
{
    if (ctx->state == rpi3StateSFPdisconnected) {
        if (sfpIsConnected(&ctx->sfp)) {
            ctx->lastHeartbeat = chTimeNow();
            ctx->state = rpi3StateSFPconnected;
        }
    } else if (ctx->state == rpi3StateSFPconnected) {
        if (!sfpIsConnected(&ctx->sfp) || (chTimeNow() - ctx->lastHeartbeat) > 15000) {
            rpi3Reset(ctx);
        }
    }
}
