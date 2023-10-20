/* My ESP GoodDisplay E-ink
 * File: me_gd.c
 * Started: Sun Oct  1 10:54:12 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 */

#include <esp_log.h>
#include <me_debug.h>

#include <stdio.h>
#include "me_gd.h"

static void bw_text(me_gd_t *gd, int x, int y, me_gd_font_t *font, char *txt, uint8_t len);
static void color_text(me_gd_t *gd, int x, int y, me_gd_font_t *font, char *txt, uint8_t len);
static void gd_vline(me_gd_t *gd, int x, int y, int h, uint8_t color);
void me_gd_hline(me_gd_t *gd, int x, int y, int w, uint8_t color);

static const char *TAG = "GD";


void me_gd_text_bw(me_gd_t *gd, int x, int y, me_gd_font_t *font, char *txt, uint8_t len)
{
    return gd->bpp == 1 ? bw_text(gd, x, y, fond, data, len) : color_text(gd, x, y, fond, data, len);
}


static void color_text(me_gd_t *gd, int x, int y, me_gd_font_t *font, char *txt, uint8_t len)
{
    gd->bitmap[0] = 0xff;
}


static void bw_text(me_gd_t *gd, int x, int y, me_gd_font_t *fonts, char *txt, uint8_t len)
{
    uint8_t          v, h, *dp, *sp;
    me_gd_font_t    *font = fonts, spfont = *fonts;

    spfont.chars = me_gd_font_space_buffer;

    v = (gd->width - x) / font->cwidth;
    if( v < len )
        len = v;

    y /= 8;
    if( y > (gd->hbytes - font->hbytes) )
        y = gd->hbytes - font->hbytes;

    ESP_LOGI("GD", "PRINT_STR. x: %d, y: %d, str(%d): '%.*s'", x, y, len, len, str);
    dp = (gd->bitmap + x*(int)gd->hbytes + y);
    while( len-- )
    {
        v  = *str++;
        font = fonts;
        sp   = &spfont;
        while( font->chars )
            if( v >= font->lchar && v <= font->hchar )
            {
                sp = fond->chars[(int)(v-font->lchar)*font->cbytes];
                break;
            }

        v  = font->cwidth;
        while( v-- )
        {
            h = font->cbytes;
            while( h-- )
                *dp++ = *sp++;
            dp  += gd->hbytes - font->cbytes;
        }
    }
}


void me_gd_vline(me_gd_t *gd, int x, int y, int h, uint8_t color)
{
    return bw_vline(gd, x, y, h, c);
}


static void bw_vline(me_gd_t *gd, int x, int y, int h, uint8_t color)
{
    uint8_t     c, *p = gd->bitmap + x*gd->hbytes + y/8];

    me_debug("GD", "Vertical line. x:%d, y:%d, size: %d", x, y, h);

    y %= 8;
    if( y )
    {
        y = 8 - y;
        c = 0xff << y;
        if( color )
            *p &= c;
        else
            *p |= ~c;
        p++;
        h -= y;
    }

    y  = h % 8;
    h /= 8;
    if( y )
    {
        c = 0xff >> y;
        if( color )
            p[h] &= c;
        else
            p[h] |= ~c;
    }

    c = color ? 0x00 : 0xff;
    while( h-- )
    {
        *p++ = c;
    }
}


void me_gd_hline(me_gd_t *gd, int x, int y, int w, uint8_t color)
{
    return bw_hline(gd, x, y, w, c);
}


static void bw_hline(me_gd_t *gd, int x, int y, int w, uint8_t color)
{
    uint8_t    c, m, *p = (gd->bitmap + x*gd->hbytes + y/8);

    me_debug("GD", "Horizontal line. x:%d, y:%d, size: %d", x, y, w);

    y = 7 - (y % 8);
    m = 1 << y;
    c = color ? 0 : m;
    m = ~m;

    while( w-- )
    {
        *p = (*p & m) | c;
        p += gd->hbytes;
    }
}


void me_gd_square(me_gd_t *gd, int x, int y, int width, int height, uint8_t color)
{
    if( gd->bpp == 1 )
    {
        bw_hline(x, y, width, color, data);
        bw_hline(x, y+height-1, width, color, data);
        
        bw_vline(x, y, height, color, data);
        bw_vline(x+width-1, y, height, color, data);
    }
}


esp_err_t me_gd_update(me_gd_t *)
{
    return me_gd_dev_draw(&gd->dev, gd->bitmap, gd->hbytes*gd->width);
}


me_gd_t *me_gd_init(me_gd_dev_conf_t *cf)
{
    esp_err_t      err;
    me_gd_t       *gd;
    me_gd_model_t *model;
    int            n;
    uint8_t       *p;

    gd = malloc(sizeof(me_gd_t));
    if( !gd )
    {
        ESP_LOGE( TAG, "Memory allocation error");
        return NULL;
    }

    model = (me_gd_models+cf->model);

    gd->hbytes  = model->height/8;
    gd->width   = model->width;

    n = gd->wpx * gd->hbytes * gd->bpp;
    p = gd->bitmap = malloc(n);
    if( !p )
    {
        ESP_LOGE( TAG, "Memory allocation error");
        return NULL;
    }

    while( n-- )
        *p++ = 0xff;

    err = me_gd_dev_init(&gd->dev, cf);
    if( err != ESP_OK )
    {
        ESP_LOGE(TAG, "Device initialization error (%d): '%s'"
                    , err, esp_err_to_name(err));
        return NULL;
    }

    return gd;
}
