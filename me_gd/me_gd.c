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
static void cl_text(me_gd_t *gd, int x, int y, me_gd_font_t *font, char *txt, uint8_t len);
static void bw_vline(me_gd_t *gd, int x, int y, int h, uint8_t color);
static void bw_hline(me_gd_t *gd, int x, int y, int w, uint8_t color);
static void cl_vline(me_gd_t *gd, int x, int y, int h, uint8_t color);
static void cl_hline(me_gd_t *gd, int x, int y, int w, uint8_t color);

static const char *TAG = "GD";


void me_gd_text(me_gd_t *gd, int x, int y, me_gd_font_t *font, char *txt, uint8_t len)/*{{{*/
{
    if( gd->bpp == 1 )
        bw_text(gd, x, y, font, txt, len);
    else
        cl_text(gd, x, y, font, txt, len);
}/*}}}*/


static void cl_text(me_gd_t *gd, int x, int y, me_gd_font_t *font, char *txt, uint8_t len)/*{{{*/
{
    gd->bitmap[0] = 0xff;
}/*}}}*/


static void bw_text(me_gd_t *gd, int x, int y, me_gd_font_t *fonts, char *txt, uint8_t len)/*{{{*/
{
    uint8_t          v, h, *dp, *sp;
    me_gd_font_t    *font = fonts;

    v = (gd->width - x) / font->cwidth;
    if( v < len )
        len = v;

    y /= 8;
    if( y > (gd->hbytes - font->hbytes) )
        y = gd->hbytes - font->hbytes;

    me_debug("GD", "PRINT_STR. x: %d, y: %d, str(%d): '%.*s'", x, y, len, len, txt);
    dp = (gd->bitmap + x*(int)gd->hbytes + y);
    while( len-- )
    {
        v  = *txt++;
        
        font = fonts;
        while( font->chars )
            if( v >= font->lchar && v <= font->hchar )
                break;
            else
                font++;

        if( font->chars )
        {
            sp = &font->chars[(int)(v - font->lchar) * font->cbytes];
        } else 
        {
            font = fonts;
            sp   = me_gd_font_space_buffer;
        }

        v  = font->cwidth;
        while( v-- )
        {
            h = font->hbytes;
            while( h-- )
                *dp++ = *sp++;
            dp  += gd->hbytes - font->hbytes;
        }
    }
}/*}}}*/


void me_gd_vline(me_gd_t *gd, int x, int y, int h, uint8_t color)/*{{{*/
{
    if( gd->bpp == 1 )
        bw_vline(gd, x, y, h, color);
    else
        cl_vline(gd, x, y, h, color);
}
/*}}}*/


static void cl_vline(me_gd_t *gd, int x, int y, int h, uint8_t color)/*{{{*/
{
    return;
}/*}}}*/


static void bw_vline(me_gd_t *gd, int x, int y, int h, uint8_t color)/*{{{*/
{
    uint8_t     c, *p = gd->bitmap + x*gd->hbytes + y/8;

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
}/*}}}*/


void me_gd_hline(me_gd_t *gd, int x, int y, int w, uint8_t color)/*{{{*/
{
    if( gd->bpp == 1 )
        bw_hline(gd, x, y, w, color);
    else
        cl_hline(gd, x, y, w, color);
}/*}}}*/


static void cl_hline(me_gd_t *gd, int x, int y, int w, uint8_t color)/*{{{*/
{
    return;
}/*}}}*/


static void bw_hline(me_gd_t *gd, int x, int y, int w, uint8_t color)/*{{{*/
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
}/*}}}*/


void me_gd_square(me_gd_t *gd, int x, int y, int width, int height, uint8_t color)/*{{{*/
{
    if( gd->bpp == 1 )
    {
        bw_hline(gd, x, y, width, color);
        bw_hline(gd, x, y+height-1, width, color);
        bw_vline(gd, x, y, height, color);
        bw_vline(gd, x+width-1, y, height, color);
    } else 
    {
        cl_hline(gd, x, y, width, color);
        cl_hline(gd, x, y+height-1, width, color);
        cl_vline(gd, x, y, height, color);
        cl_vline(gd, x+width-1, y, height, color);
    }
}/*}}}*/


void me_gd_box(me_gd_t *gd, int x, int y, int width, int height, uint8_t color)/*{{{*/
{
    if( gd->bpp == 1 )
    {
        while( width-- )
            bw_vline(gd, x++, y, height, color);
    } else 
    {
        while( width-- )
           cl_vline(gd, x++, y, height, color);
    }
}/*}}}*/


void me_gd_invert(me_gd_t *gd, int x, int y, int width, int height)/*{{{*/
{
    uint8_t     *dp, h, c;

    me_debug("GD", "Invert x: %d, y: %d, w: %d, h: %d", x, y, width, height);

    y      /= 8;
    height /= 8;
    while( width-- )
    {
        dp = (gd->bitmap + (x++ * gd->hbytes) + y);
        h  = height;
        while( h-- )
        {
            c = *dp;
            *dp++ = ~(c);
        }
    }
}/*}}}*/


void me_gd_num(me_gd_t *gd, int x, int y, me_gd_font_t *font, int num)/*{{{*/
{
    char        buf[32], *p;
    uint8_t     is_neg, n=0;

    if( num < 0 )
    {
        num    *= (-1);
        is_neg = 1;
    } else 
        is_neg = 0;

    p = buf + sizeof(buf) - 1;

    do {
        n++;
        *p-- = '0' + num % 10;
        num /= 10;
    } while( num );

    if( is_neg )
        *p = '-';
    else
        p++;
    
    me_gd_text(gd, x, y, font, p, n);
}/*}}}*/


void me_gd_image(me_gd_t *gd, int x, int y, int width, uint8_t hbytes, uint8_t *sp)/*{{{*/
{
    uint8_t  h, *dp;

    if( x + width > gd->width )
        return;

    y /= 8;
    if( y + hbytes >  gd->hbytes )
        return;

me_debug("GD", "Draw image at x: %d, y: %d, w: %d, h: %d"
             , x, y, width, hbytes);
    while( width-- )
    {
        dp = gd->bitmap + (x++)*gd->hbytes + y;
        h = hbytes;
        while( h-- > 0 )
        {
            *dp++ = *sp++;
        }
    }
}/*}}}*/


inline esp_err_t me_gd_update(me_gd_t *gd)/*{{{*/
{
    return me_gd_dev_draw(&gd->dev, gd->bitmap, gd->hbytes*gd->width);
}/*}}}*/


inline esp_err_t me_gd_update_fast(me_gd_t *gd)/*{{{*/
{
    return me_gd_dev_draw_fast(&gd->dev, gd->bitmap, gd->hbytes*gd->width);
}/*}}}*/


me_gd_t *me_gd_init(me_gd_dev_conf_t *cf)/*{{{*/
{
    esp_err_t      err;
    me_gd_t       *gd;
    me_gd_model_t *model;
    int            n;
    uint8_t       *p;

    gd = malloc(sizeof(me_gd_t));
    if( !gd )
    {
        ESP_LOGE( TAG, "Cannot allocate %d bytes for me_gd_t"
                     , sizeof(me_gd_t));
        return NULL;
    }

    model = (me_gd_models+cf->model);

    gd->hbytes  = model->height/8;
    gd->width   = model->width;
    gd->bpp     = model->bpp;

    n = gd->width * gd->hbytes * gd->bpp;
    p = gd->bitmap = malloc(n);
    if( !p )
    {
        ESP_LOGE( TAG, "Cannot allocate %d bytes for bitmap", n);
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

    n = ME_GD_FONT_MAX_CBYTES;
    p = me_gd_font_space_buffer;
    while( n-- )
        *p++ = me_gd_bw_white;

    return gd;
}/*}}}*/
