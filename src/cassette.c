// -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; st-rulers: [132] -*-
// vim: ts=4 sw=4 ft=c++ et

#include "cassette.h"

#define CASSETTE_MAX 1

uint8_t
cassetteCount(void)
{
    user_param *fp = NULL;
    fp = cassetteOpenRead(0);
    if (fp == NULL) {
        return 0;
    }
    return 1;
}

uint8_t
cassetteFree(void)
{
    return (CASSETTE_MAX - cassetteCount());
}

uint8_t
cassetteMax(void)
{
    return CASSETTE_MAX;
}

user_param *
cassetteOpenWrite(uint8_t index)
{
    user_param *fp = NULL;
    if (index >= CASSETTE_MAX) {
        return NULL;
    }
    fp = vexFlashUserParamRead();
    fp->data[0] = 0;
    return fp;
}

user_param *
cassetteOpenRead(uint8_t index)
{
    user_param *fp = NULL;
    if (index >= CASSETTE_MAX) {
        return NULL;
    }
    fp = vexFlashUserParamRead();
    if (fp->data[0] == 0 || fp->data[0] > 31) {
        return NULL;
    }
    return fp;
}
