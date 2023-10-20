/* My ESP string functions
 * File: me_str.h
 * Started: Wed Oct 20 08:50:53 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */

#include <esp_system.h>
#include <sys/types.h>

#ifndef _ME_STR_H
#define _ME_STR_H

#define me_str(X)       { (u_char *)(X), sizeof(X) - 1 }
#define me_str_null     { NULL, 0 }

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

/**
 * @brief Convert atoi with buf len
 * @param buf buffer 
 * @param len text length
 * @return signed integer 32bit
 */
int32_t me_atoi(unsigned char *buf, size_t len);

/**
 * @brief Convert atoi with buf len
 * @param buf buffer 
 * @param len text length
 * @return double
 */
double me_atod(unsigned char *buf, size_t len);

#endif
