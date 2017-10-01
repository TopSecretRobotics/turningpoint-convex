// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et
/*
 * messages.h
 */

#ifndef MESSAGES_H_

#define MESSAGES_H_

#include <stdint.h>
#include <string.h>

#define MESSAGES_OP_PING 0x01
#define MESSAGES_OP_PONG 0x02
#define MESSAGES_OP_INFO 0x03
#define MESSAGES_OP_DATA 0x04
#define MESSAGES_OP_READ 0x05
#define MESSAGES_OP_WRITE 0x06
#define MESSAGES_OP_SUBSCRIBE 0x07
#define MESSAGES_OP_UNSUBSCRIBE 0x08

#define MESSAGES_DATA_FLAG_END 0x01
#define MESSAGES_DATA_FLAG_PUB 0x02
#define MESSAGES_DATA_FLAG_ERROR 0x04

#define MESSAGES_ERROR_BAD_REQ_ID 0x01
#define MESSAGES_ERROR_BAD_TOPIC 0x02
#define MESSAGES_ERROR_BAD_SUBTOPIC 0x03
#define MESSAGES_ERROR_SUB_MAX 0x04

#define MESSAGES_TOPIC_PUBSUB 0x00
#define MESSAGES_TOPIC_PUBSUB_SUBTOPIC_COUNT 0xfb
#define MESSAGES_TOPIC_PUBSUB_SUBTOPIC_FREE 0xfc
#define MESSAGES_TOPIC_PUBSUB_SUBTOPIC_MAX 0xfd
#define MESSAGES_TOPIC_PUBSUB_SUBTOPIC_LIST 0xfe
#define MESSAGES_TOPIC_PUBSUB_SUBTOPIC_ALL 0xff
#define MESSAGES_TOPIC_CLOCK 0x01
#define MESSAGES_TOPIC_CLOCK_SUBTOPIC_NOW 0x00
#define MESSAGES_TOPIC_MOTOR 0x02
#define MESSAGES_TOPIC_MOTOR_SUBTOPIC_ALL 0xff
#define MESSAGES_TOPIC_SMARTMOTOR 0x03
#define MESSAGES_TOPIC_SMARTMOTOR_SUBTOPIC_ALL 0xff
#define MESSAGES_TOPIC_NETWORK 0x04
#define MESSAGES_TOPIC_NETWORK_SUBTOPIC_IPV4 0x00
#define MESSAGES_TOPIC_ROBOT 0x05
#define MESSAGES_TOPIC_ROBOT_SUBTOPIC_SPI 0x00
#define MESSAGES_TOPIC_CASSETTE 0x06
#define MESSAGES_TOPIC_CASSETTE_SUBTOPIC_WRITE 0xf8
#define MESSAGES_TOPIC_CASSETTE_SUBTOPIC_CLOSE 0xf9
#define MESSAGES_TOPIC_CASSETTE_SUBTOPIC_OPEN 0xfa
#define MESSAGES_TOPIC_CASSETTE_SUBTOPIC_COUNT 0xfb
#define MESSAGES_TOPIC_CASSETTE_SUBTOPIC_FREE 0xfc
#define MESSAGES_TOPIC_CASSETTE_SUBTOPIC_MAX 0xfd
#define MESSAGES_TOPIC_ALL 0xff
#define MESSAGES_TOPIC_ALL_SUBTOPIC_ALL 0xff

typedef struct message_s {
    uint8_t op;
} message_t;

typedef struct message_ping_s {
    uint8_t op;
    uint8_t seq_id;
} message_ping_t;

typedef struct message_pong_s {
    uint8_t op;
    uint8_t seq_id;
} message_pong_t;

typedef struct message_info_s {
    uint8_t op;
    uint8_t topic;
    uint8_t subtopic;
    uint8_t len;
    uint8_t *value;
} message_info_t;

typedef struct message_req_s {
    uint8_t op;
    uint16_t req_id;
} message_req_t;

typedef struct message_data_s {
    uint8_t op;
    uint16_t req_id;
    uint8_t topic;
    uint8_t subtopic;
    uint8_t flag;
    uint32_t timestamp;
    uint8_t len;
    uint8_t *value;
} message_data_t;

typedef struct message_read_s {
    uint8_t op;
    uint16_t req_id;
    uint8_t topic;
    uint8_t subtopic;
} message_read_t;

typedef struct message_write_s {
    uint8_t op;
    uint16_t req_id;
    uint8_t topic;
    uint8_t subtopic;
    uint8_t len;
    uint8_t *value;
} message_write_t;

typedef struct message_subscribe_s {
    uint8_t op;
    uint16_t req_id;
    uint8_t topic;
    uint8_t subtopic;
} message_subscribe_t;

typedef struct message_unsubscribe_s {
    uint8_t op;
    uint16_t req_id;
} message_unsubscribe_t;

typedef union message_any_t {
    message_t message;
    message_ping_t ping;
    message_pong_t pong;
    message_info_t info;
    message_req_t req;
    message_data_t data;
    message_read_t read;
    message_write_t write;
    message_subscribe_t subscribe;
    message_unsubscribe_t unsubscribe;
} message_any_t;

#ifdef __cplusplus
extern "C" {
#endif

extern void message_ping_frame(message_ping_t *message, uint8_t seq_id);
extern void message_pong_frame(message_pong_t *message, uint8_t seq_id);
extern void message_info_frame(message_info_t *message, uint8_t topic, uint8_t subtopic, uint8_t len, uint8_t *value);
extern void message_data_frame(message_data_t *message, uint16_t req_id, uint8_t topic, uint8_t subtopic, uint8_t flag,
                               uint32_t timestamp, uint8_t len, uint8_t *value);
extern void message_read_frame(message_read_t *message, uint16_t req_id, uint8_t topic, uint8_t subtopic);
extern void message_write_frame(message_write_t *message, uint16_t req_id, uint8_t topic, uint8_t subtopic, uint8_t len,
                                uint8_t *value);
extern void message_subscribe_frame(message_subscribe_t *message, uint16_t req_id, uint8_t topic, uint8_t subtopic);
extern void message_unsubscribe_frame(message_unsubscribe_t *message, uint16_t req_id);
extern size_t message_getsizeof(const message_any_t *m);
extern int message_serialize(const message_any_t *m, uint8_t *buf, size_t len, size_t *outlen);
extern int message_deserialize(message_any_t *m, const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif
