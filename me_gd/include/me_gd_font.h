/* My ESP GoodDisplay E-ink font
 * File: me_gd_font.h
 * Started: Mon Oct  2 09:33:57 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 */

#ifndef _ME_GD_FONT_H
#define _ME_GD_FONT_H

#include <stdint.h>

/* NOTE: font chars is a bitmap of chars from lchar to hchar */
#define ME_GD_FONT_MAX_CBYTES       (32*40/8)
#define me_gd_font_char_ext_begin   224
#define me_gd_font_char_ext_end     255

extern uint8_t me_gd_font_space_buffer[];

typedef struct 
{
    uint8_t    *chars;
    uint8_t     lchar;
    uint8_t     hchar;
    uint8_t     hbytes;
    uint8_t     cwidth;
    uint8_t     cbytes;
} me_gd_font_t;

extern me_gd_font_t me_gd_font_full16[];
extern me_gd_font_t me_gd_font_num32[];
extern me_gd_font_t me_gd_font_num24[];
extern me_gd_font_t me_gd_font_num40[];

#endif

