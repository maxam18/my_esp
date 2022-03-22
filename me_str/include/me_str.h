/* My ESP string functions
 * File: me_str.h
 * Started: Wed Oct 20 08:50:53 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */

#include <sys/types.h>

#ifndef _ME_STR_H
#define _ME_STR_H

#define me_str(X)  { (u_char *)X, sizeof(X) - 1 }

/**
 * @brief String holder structure
 */
typedef struct {
    u_char    *data;  /*!< data pointer */
    size_t     len;   /*!< string length */
} me_str_t;

/**
 * @brief Buffer structure
 *        Useful in constructing long strings
 */
typedef struct {
    u_char    *start; /*!< allocated buffer address pointer */
    u_char    *pos;   /*!< current operation position */
    u_char    *end;   /*!< allocated buffer end address pointer */
} me_str_buf_t;

#endif
