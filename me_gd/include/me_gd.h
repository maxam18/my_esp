/* My ESP GoodDisplay E-ink
 * File: me_gd.h
 * Started: Sun Oct  1 10:54:18 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 */

#ifndef _ME_GD_H
#define _ME_GD_H

#include <esp_err.h>
#include <me_gd_dev.h>
#include <me_gd_model.h>
#include <me_gd_font.h>

//2bit
#define me_gd_color_black   0x00  /// 00
#define me_gd_color_white   0x01  /// 01
#define me_gd_color_yellow  0x02  /// 10
#define me_gd_color_red     0x03  /// 11

#define me_gd_bw_white      0xff
#define me_gd_bw_black      0x00

typedef struct me_gd_s
{
    uint8_t                 bpp;
    uint8_t                 hbytes;
    int                     width;

    me_gd_dev_t             dev;
    uint8_t                *bitmap;
} me_gd_t;

/**
 * \brief draw text on bitmap with font specified
 * NOTE: font MUST end with the space char!
 */
void me_gd_text(me_gd_t *gd, int x, int y, me_gd_font_t *font, char *txt, uint8_t len);

void me_gd_num(me_gd_t *gd, int x, int y, me_gd_font_t *font, int num);

me_gd_t *me_gd_init(me_gd_dev_conf_t *);

esp_err_t me_gd_update(me_gd_t *);
esp_err_t me_gd_update_fast(me_gd_t *);

void me_gd_hline(me_gd_t *gd, int x, int y, int w, uint8_t color);
void me_gd_vline(me_gd_t *gd, int x, int y, int h, uint8_t color);
void me_gd_square(me_gd_t *gd, int x, int y, int width, int height, uint8_t color);
void me_gd_invert(me_gd_t *gd, int x, int y, int width, int height);
void me_gd_invert_part(me_gd_t *gd, int x, int y, int width, int height);
void me_gd_image(me_gd_t *gd, int x, int y, int cwidth, uint8_t hbytes, uint8_t *data);
void me_gd_box(me_gd_t *gd, int x, int y, int width, int height, uint8_t color);
void me_gd_image_set(me_gd_t *gd, int x, int y, int width, uint8_t hbytes, uint8_t *sp);

#endif