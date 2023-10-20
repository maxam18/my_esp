/* My ESP GoodDisplay E-ink model
 * File: me_gd_models.c
 * Started: Sun Oct  1 10:57:28 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 */

#include <me_gd_dev.h>

/* NOTE: width for image is a height for device, same for height */
me_gd_model_t me_gd_models[] = {
    {// 0213F51
        .bpp        = 2,
        .busy_level = 0,
        .width      = 250,
        .height     = 128
    },
    {// 029F51
        .bpp        = 2,
        .busy_level = 0,
        .width      = 296,
        .height     = 128
    },
    {// 0213B74
        .bpp        = 1,
        .busy_level = 1,
        .width      = 250,
        .height     = 128
    }
};

static me_gd_dev_cmd_t gdey0213F51_init[] = {
    { 0x4D, me_str("\x78")},
    { 0x00, me_str("\x0F\x29")}, // PSR resolution
    { 0x01, me_str("\x07\x00")}, // PWRR
    { 0x03, me_str("\x00\x54\x44")}, // POFS
    { 0x06, me_str("\x05\x00\x3f\x0a\x25\x12\x1a")}, // BTST_P
    { 0x30, me_str("\x07")}, // PLL
    { 0x50, me_str("\x37")}, // CDI / gray1
    { 0x60, me_str("\x02\x02")}, // TCON
    { 0x61, me_str("\x00\x80\x00\xFA")}, // TRES : width/256,width%256,height/256,height%256
    { 0xe7, me_str("\x1c")}, 
    { 0xe3, me_str("\x22")}, 
    { 0xB4, me_str("\xd0")}, 
    { 0xB5, me_str("\x03")}, 
    { 0xe9, me_str("\x01")}, 
    { 0x30, me_str("\x08")}, // PLL control: enabled 12kHz
    { 0x04, me_str_null}, // BUSY_N control
    me_gd_dev_cmd_null
};
static me_gd_dev_cmd_t gdey029F51_init[] = {
    { 0x4D, me_str("\x78")},
    { 0x00, me_str("\x8F\x29")}, // PSR resolution
    { 0x01, me_str("\x07\x00")}, // PWRR
    { 0x03, me_str("\x00\x54\x44")}, // POFS
    { 0x06, me_str("\x05\x00\x3f\x0a\x25\x12\x1a")}, // BTST_P
    { 0x30, me_str("\x07")}, // PLL
    { 0x50, me_str("\x37")}, // CDI / gray1
    { 0x60, me_str("\x02\x02")}, // TCON
    { 0x61, me_str("\x00\x80\x01\x28")}, // TRES : width/256,width%256,height/256,height%256
    { 0xe7, me_str("\x1c")}, 
    { 0xe3, me_str("\x22")}, 
    { 0xB4, me_str("\xd0")}, 
    { 0xB5, me_str("\x03")}, 
    { 0xe9, me_str("\x01")}, 
    { 0x30, me_str("\x08")}, // PLL control: enabled 12kHz
    { 0x04, me_str_null}, // BUSY_N control
    me_gd_dev_cmd_null
};
static me_gd_dev_cmd_t gdey0213B74_init[] = {
    { 0x01, me_str("\xF9\x00\x00")}, // (height-1)%256,height-1)/256,00
    { 0x11, me_str("\x01")},
    { 0x44, me_str("\x00\x0F")}, // 00,width/8-1
    { 0x45, me_str("\xF9\x00\x00\x00")}, // (height-1)%256,height-1)/256
    { 0x3C, me_str("\x05")},
    { 0x21, me_str("\x00\x80")}, // inverse, available source
    { 0x18, me_str("\x80")},
    { 0x4E, me_str("\x00")},
    { 0x4F, me_str("\xF9\x00")}, // (height-1)%256,height-1)/256
    { 0x22, me_str("\xf7")}, // update control sequence
    me_gd_dev_cmd_null
};

me_gd_dev_cmd_set_t me_gd_models_cmd_set[] = {
        { // 0213F51
            .poweroff = { 0X02, me_str_null },
            .refresh  = { 0x12, me_str("\x00") },
            .reset    = me_gd_dev_cmd_null,
            .sleep    = { 0x07, me_str("\xa5") },
            .update   = { 0x10, me_str_null },
            .init     = gdey0213F51_init
        },
        { // 029F51
            .poweroff = { 0X02, me_str_null },
            .refresh  = { 0x12, me_str("\x00") },
            .reset    = me_gd_dev_cmd_null,
            .sleep    = { 0x07, me_str("\xa5") },
            .update   = { 0x10, me_str_null },
            .init     = gdey029F51_init
        },
        { // 0213B74
            .poweroff = me_gd_dev_cmd_null,
            .refresh  = { 0x20, me_str_null },
            .reset    = { 0x12, me_str_null },
            .sleep    = { 0x10, me_str("\x01")},
            .update   = { 0x24, me_str_null },
            .init     = gdey0213B74_init
        },
};
