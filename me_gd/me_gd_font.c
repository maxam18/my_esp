/* My ESP GoodDisplay E-ink font
 * File: me_gd_font.c
 * Started: Mon Oct  2 09:34:07 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 */

#include <me_gd_font.h>

#include <me_gd_font_num40.h>
#include <me_gd_font_num24.h>
#include <me_gd_font_num32.h>
#include <me_gd_font_num16.h>
#include <me_gd_font_ru16.h>
#include <me_gd_font_en16.h>

me_gd_font_t me_gd_font_num40[] = {
    {
        .cbytes = font_num40_cwidth*font_num40_cheight,
        .chars  = font_num40_bits,
        .cwidth = font_num40_cwidth,
        .hbytes = font_num40_cheight,
        .hchar  = '9',
        .lchar  = '0'
    },
    {
        .chars  = 0
    }
};

me_gd_font_t me_gd_font_num32[] = {
    {
        .cbytes = font_num32_cwidth*font_num32_cheight,
        .chars  = font_num32_bits,
        .cwidth = font_num32_cwidth,
        .hbytes = font_num32_cheight,
        .hchar  = '9',
        .lchar  = '0'
    },
    {
        .chars  = 0
    }
};

me_gd_font_t me_gd_font_num24[] = {
    {
        .cbytes = font_num24_cwidth*font_num24_cheight,
        .chars  = font_num24_bits,
        .cwidth = font_num24_cwidth,
        .hbytes = font_num24_cheight,
        .hchar  = '9',
        .lchar  = '0'
    },
    {
        .chars  = 0
    }
};

me_gd_font_t me_gd_font_full16[] = {
    {
        .cbytes = font_num16_cwidth*font_num16_cheight,
        .chars  = font_num16_bits,
        .cwidth = font_num16_cwidth,
        .hbytes = font_num16_cheight,
        .hchar  = '9',
        .lchar  = '0'
    },
    {
        .cbytes = font_ru16_cwidth*font_ru16_cheight,
        .chars  = font_ru16_bits,
        .cwidth = font_ru16_cwidth,
        .hbytes = font_ru16_cheight,
        .hchar  = me_gd_font_char_ext_end,
        .lchar  = me_gd_font_char_ext_begin
    },
    {
        .cbytes = font_en16_cwidth*font_en16_cheight,
        .chars  = font_en16_bits,
        .cwidth = font_en16_cwidth,
        .hbytes = font_en16_cheight,
        .hchar  = 'z',
        .lchar  = 'a'
    },
    {
        .chars = 0
    }
};

uint8_t me_gd_font_space_buffer[ME_GD_FONT_MAX_CBYTES] = {0xff};
