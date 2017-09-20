// -*- mode: c; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c et

#include "messages.h"
#include "portable_endian.h"

void
message_ping_frame(message_ping_t *message, uint8_t seq_id)
{
    message->op = MESSAGES_OP_PING;
    message->seq_id = seq_id;
}

void
message_pong_frame(message_pong_t *message, uint8_t seq_id)
{
    message->op = MESSAGES_OP_PONG;
    message->seq_id = seq_id;
}

void
message_info_frame(message_info_t *message, uint8_t len, uint8_t *value)
{
    message->op = MESSAGES_OP_INFO;
    message->len = len;
    message->value = value;
}

void
message_data_frame(message_data_t *message, uint8_t req_id, uint8_t flag, uint8_t len, uint8_t *value)
{
    message->op = MESSAGES_OP_DATA;
    message->req_id = req_id;
    message->flag = flag;
    message->len = len;
    message->value = value;
}

void
message_read_frame(message_read_t *message, uint8_t req_id, uint8_t type, uint8_t topic)
{
    message->op = MESSAGES_OP_READ;
    message->req_id = req_id;
    message->type = type;
    message->topic = topic;
}

void
message_write_frame(message_write_t *message, uint8_t req_id, uint8_t type, uint8_t topic, uint8_t len, uint8_t *value)
{
    message->op = MESSAGES_OP_WRITE;
    message->req_id = req_id;
    message->type = type;
    message->topic = topic;
    message->len = len;
    message->value = value;
}

void
message_subscribe_frame(message_subscribe_t *message, uint8_t req_id, uint8_t type, uint8_t topic)
{
    message->op = MESSAGES_OP_SUBSCRIBE;
    message->req_id = req_id;
    message->type = type;
    message->topic = topic;
}

void
message_unsubscribe_frame(message_unsubscribe_t *message, uint8_t req_id)
{
    message->op = MESSAGES_OP_UNSUBSCRIBE;
    message->req_id = req_id;
}

void
message_publish_frame(message_publish_t *message, uint8_t req_id, uint8_t type, uint8_t topic, uint8_t len, uint8_t *value)
{
    message->op = MESSAGES_OP_PUBLISH;
    message->req_id = req_id;
    message->type = type;
    message->topic = topic;
    message->len = len;
    message->value = value;
}

size_t
message_getsizeof(const message_any_t *m)
{
    size_t mlen = 1; // message_t.op
    switch (m->message.op) {
    case MESSAGES_OP_PING:
        mlen += 1; // message_ping_t.seq_id
        break;
    case MESSAGES_OP_PONG:
        mlen += 1; // message_pong_t.seq_id
        break;
    case MESSAGES_OP_INFO:
        mlen += 1; // message_info_t.len
        mlen += m->info.len;
        break;
    case MESSAGES_OP_DATA:
        mlen += 2; // message_req_t.req_id
        mlen += 1; // message_data_t.flag
        mlen += 1; // message_data_t.len
        mlen += m->data.len;
        break;
    case MESSAGES_OP_READ:
        mlen += 2; // message_req_t.req_id
        mlen += 1; // message_read_t.type
        mlen += 1; // message_read_t.topic
        break;
    case MESSAGES_OP_WRITE:
        mlen += 2; // message_req_t.req_id
        mlen += 1; // message_write_t.type
        mlen += 1; // message_write_t.topic
        mlen += 1; // message_write_t.len
        mlen += m->write.len;
        break;
    case MESSAGES_OP_SUBSCRIBE:
        mlen += 2; // message_req_t.req_id
        mlen += 1; // message_subscribe_t.type
        mlen += 1; // message_subscribe_t.topic
        break;
    case MESSAGES_OP_UNSUBSCRIBE:
        mlen += 2; // message_req_t.req_id
        break;
    case MESSAGES_OP_PUBLISH:
        mlen += 2; // message_req_t.req_id
        mlen += 1; // message_publish_t.type
        mlen += 1; // message_publish_t.topic
        mlen += 1; // message_publish_t.len
        mlen += m->publish.len;
        break;
    default:
        return 0;
    }
    return mlen;
}

int
message_serialize(const message_any_t *m, uint8_t *buf, size_t len, size_t *outlen)
{
    size_t mlen = message_getsizeof(m);
    uint16_t req_id;
    if (mlen == 0 || mlen > len) {
        return -1;
    }
    switch (m->message.op) {
    case MESSAGES_OP_PING:
        buf[0] = m->ping.op;
        buf[1] = m->ping.seq_id;
        break;
    case MESSAGES_OP_PONG:
        buf[0] = m->pong.op;
        buf[1] = m->pong.seq_id;
        break;
    case MESSAGES_OP_INFO:
        buf[0] = m->info.op;
        buf[1] = m->info.len;
        (void)memcpy(buf + 2, m->info.value, m->info.len);
        break;
    case MESSAGES_OP_DATA:
        buf[0] = m->data.op;
        req_id = (uint16_t)(htons(m->data.req_id));
        (void)memcpy(buf + 1, &req_id, 2);
        buf[3] = m->data.flag;
        buf[4] = m->data.len;
        (void)memcpy(buf + 5, m->data.value, m->data.len);
        break;
    case MESSAGES_OP_READ:
        buf[0] = m->read.op;
        req_id = (uint16_t)(htons(m->read.req_id));
        (void)memcpy(buf + 1, &req_id, 2);
        buf[3] = m->read.type;
        buf[4] = m->read.topic;
        break;
    case MESSAGES_OP_WRITE:
        buf[0] = m->write.op;
        req_id = (uint16_t)(htons(m->write.req_id));
        (void)memcpy(buf + 1, &req_id, 2);
        buf[3] = m->write.type;
        buf[4] = m->write.topic;
        buf[5] = m->write.len;
        (void)memcpy(buf + 6, m->write.value, m->write.len);
        break;
    case MESSAGES_OP_SUBSCRIBE:
        buf[0] = m->subscribe.op;
        req_id = (uint16_t)(htons(m->subscribe.req_id));
        (void)memcpy(buf + 1, &req_id, 2);
        buf[3] = m->subscribe.type;
        buf[4] = m->subscribe.topic;
        break;
    case MESSAGES_OP_UNSUBSCRIBE:
        buf[0] = m->unsubscribe.op;
        req_id = (uint16_t)(htons(m->unsubscribe.req_id));
        (void)memcpy(buf + 1, &req_id, 2);
        break;
    case MESSAGES_OP_PUBLISH:
        buf[0] = m->publish.op;
        req_id = (uint16_t)(htons(m->publish.req_id));
        (void)memcpy(buf + 1, &req_id, 2);
        buf[3] = m->publish.type;
        buf[4] = m->publish.topic;
        buf[5] = m->publish.len;
        (void)memcpy(buf + 6, m->publish.value, m->publish.len);
        break;
    default:
        return -1;
    }
    if (outlen) {
        *outlen = mlen;
    }
    return 0;
}

int
message_deserialize(message_any_t *m, const uint8_t *buf, size_t len)
{
    uint16_t req_id;
    uint8_t vlen;
    if (len < 2) {
        return -1;
    }
    switch (buf[0]) {
    case MESSAGES_OP_PING:
        m->ping.op = buf[0];
        m->ping.seq_id = buf[1];
        break;
    case MESSAGES_OP_PONG:
        m->pong.op = buf[0];
        m->pong.seq_id = buf[1];
        break;
    case MESSAGES_OP_INFO:
        vlen = buf[1];
        vlen += 2;
        if (len < vlen) {
            return -1;
        }
        m->info.op = buf[0];
        m->info.len = buf[1];
        m->info.value = (uint8_t *)(buf + 2);
        break;
    case MESSAGES_OP_DATA:
        vlen = buf[4];
        vlen += 5;
        if (len < 5 || len < vlen) {
            return -1;
        }
        m->data.op = buf[0];
        (void)memcpy(&req_id, buf + 1, 2);
        m->data.req_id = (uint16_t)(ntohs(req_id));
        m->data.flag = buf[3];
        m->data.len = buf[4];
        m->data.value = (uint8_t *)(buf + 5);
        break;
    case MESSAGES_OP_READ:
        if (len < 5) {
            return -1;
        }
        m->read.op = buf[0];
        (void)memcpy(&req_id, buf + 1, 2);
        m->read.req_id = (uint16_t)(ntohs(req_id));
        m->read.type = buf[3];
        m->read.topic = buf[4];
        break;
    case MESSAGES_OP_WRITE:
        vlen = buf[5];
        vlen += 6;
        if (len < 6 || len < vlen) {
            return -1;
        }
        m->write.op = buf[0];
        (void)memcpy(&req_id, buf + 1, 2);
        m->write.req_id = (uint16_t)(ntohs(req_id));
        m->write.type = buf[3];
        m->write.topic = buf[4];
        m->write.len = buf[5];
        m->write.value = (uint8_t *)(buf + 6);
        break;
    case MESSAGES_OP_SUBSCRIBE:
        if (len < 5) {
            return -1;
        }
        m->subscribe.op = buf[0];
        (void)memcpy(&req_id, buf + 1, 2);
        m->subscribe.req_id = (uint16_t)(ntohs(req_id));
        m->subscribe.type = buf[3];
        m->subscribe.topic = buf[4];
        break;
    case MESSAGES_OP_UNSUBSCRIBE:
        if (len < 3) {
            return -1;
        }
        m->unsubscribe.op = buf[0];
        (void)memcpy(&req_id, buf + 1, 2);
        m->unsubscribe.req_id = (uint16_t)(ntohs(req_id));
        break;
    case MESSAGES_OP_PUBLISH:
        vlen = buf[5];
        vlen += 6;
        if (len < 6 || len < vlen) {
            return -1;
        }
        m->publish.op = buf[0];
        (void)memcpy(&req_id, buf + 1, 2);
        m->publish.req_id = (uint16_t)(ntohs(req_id));
        m->publish.type = buf[3];
        m->publish.topic = buf[4];
        m->publish.len = buf[5];
        m->publish.value = (uint8_t *)(buf + 6);
        break;
    default:
        return -1;
    }
    return 0;
}
