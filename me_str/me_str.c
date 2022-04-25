/* My ESP string functions
 * File: me_str.c
 * Started: Wed Oct 20 08:50:32 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */

#include "me_str.h"

int32_t me_atoi(unsigned char *buf, size_t len)
{
    int32_t     ret = 0;
    uint8_t     ch, is_neg = 0;

    if( *buf == '-' )
        is_neg = 1;

    while( len-- )
    {
        ch = *buf++;
        if( ch >= '0' && ch <= '9' )
            ret = ret * 10 + (ch - '0');
    }

    if( is_neg )
        ret *= (-1);

    return ret;
}


double me_atod(unsigned char *buf, size_t len)
{
    uint8_t    ch, is_neg = 0;
    int32_t    i32 = 0;
    double     ret = 0;

    if( *buf == '-' )
        is_neg = 1;

    for(; len; len-- )
    {
        ch = *buf++;
        if( ch == '.' )
            break;

        if( ch >= '0' && ch <= '9' )
            i32 = i32 * 10 + (ch - '0');
    }

    buf = buf + len - 1;
    for(; len; len--)
    {
        ch = *buf--;

        if( ch < '0' || ch > '9' )
            continue;

        ret += (double)(ch-'0');
        ret /= 10;
    }

    ret += i32;

    if( is_neg )
        ret *= (-1);

    return ret;
}
