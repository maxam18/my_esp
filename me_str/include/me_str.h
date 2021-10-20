/* My ESP string functions
 * File: me_str.h
 * Started: Wed Oct 20 08:50:53 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */
#include <stdint.h>

#ifndef _ME_STR_H
#define _ME_STR_H

typedef struct {
    u_char    *data;
    size_t     len;
} me_str_t;

typedef struct {
    u_char    *start;
    u_char    *pos;
    u_char    *end;
} me_str_buf_t;

#endif
