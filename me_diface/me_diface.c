/* My ESP digits display interface
 * File: me_diface.c
 * Started: Wed Aug 30 20:24:21 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 */
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_err.h>

#include <me_debug.h>
#include <me_ncoder.h>
#include <me_tm1637.h>

#include <me_diface.h>

#include "sdkconfig.h"

#define INACTIVE_DELAY          pdMS_TO_TICKS(5000)
#define FRAC_MULT               1000
#define ME_DIFACE_MAX_SHIFT     10
/*
#ifdef CONFIG_ME_DIFACE_HAS_SECOND_DISPLAY
  static me_tm1637_led_t      *disp_two;
  #define DISP_TWO_NUM(I)    me_tm1637_set_float(disp_two, (I)->value, (I)->mdot, 0)
  #define DISP_TWO_STR(V)    me_tm1637_set_text(disp_two, (V)->data, (V)->len)
  #define DISP_TWO_RESET     me_tm1637_reset(disp_two); \
                             me_tm1637_set_brightness(disp_two, CONFIG_ME_DIFACE_DISP_BRIGHTNESS);
  #ifdef CONFIG_ME_DIFACE_DISP_TWO_6SEGS
    #define ME_DIFACE_DISP_TWO_SEGS     TM1637_6SEGS_DM
  #else
    #define ME_DIFACE_DISP_TWO_SEGS     TM1637_4SEGS
  #endif
#else
#define DISP_TWO_NUM(X)
#define DISP_TWO_STR(X)
#define DISP_TWO_RESET
#endif
#define DISP_ONE_NUM(I)    me_tm1637_set_float(disp_one, (I)->value, (I)->mdot, 0)
#define DISP_ONE_STR(V)    me_tm1637_set_text(disp_one, (V)->data, (V)->len)
#define DISP_ONE_RESET     me_tm1637_reset(disp_one); \
                           me_tm1637_set_brightness(disp_one, CONFIG_ME_DIFACE_DISP_BRIGHTNESS);
#ifdef CONFIG_ME_DIFACE_DISP_ONE_6SEGS
  #define ME_DIFACE_DISP_ONE_SEGS     TM1637_6SEGS_DM
#else
  #define ME_DIFACE_DISP_ONE_SEGS     TM1637_4SEGS
#endif
*/

void me_diface_display(me_tm1637_led_t *disp, u_char *p, int len, double val, int shift);

#ifdef CONFIG_ME_DIFACE_HAS_SECOND_DISPLAY
  static me_tm1637_led_t      *disp_two;
  #define DISP_TWO_NUM(I)    me_tm1637_set_float(disp_two, (I)->value, (I)->mdot, 0)
  #define DISP_TWO_STR(V)    me_tm1637_set_text(disp_two, (V)->data, (V)->len)
  #define DISP_TWO_RESET     me_tm1637_reset(disp_two); \
                             me_tm1637_set_brightness(disp_two, CONFIG_ME_DIFACE_DISP_BRIGHTNESS);
  #ifdef CONFIG_ME_DIFACE_DISP_TWO_6SEGS
    #define ME_DIFACE_DISP_TWO_SEGS     TM1637_6SEGS_DM
  #else
    #define ME_DIFACE_DISP_TWO_SEGS     TM1637_4SEGS
  #endif
#else
#define DISP_TWO_NUM(X)
#define DISP_TWO_STR(X)
#define DISP_TWO_RESET
#endif

#define DISP_ONE_NUM(I)    me_diface_display(disp_one, NULL, 0, (I)->value, (I)->vshift)
#define DISP_ONE_ITEM(I)   me_diface_display(disp_one, (I)->name->data, (I)->name->len, 0, (I)->nshift)
//#define DISP_ONE_STRM(V,M)  me_diface_display(disp_one, (V), 0, M)
#define DISP_ONE_STR(V)    me_diface_display(disp_one, (V)->data, (V)->len, 0, 0)
#define DISP_ONE_RESET     me_tm1637_reset(disp_one); \
                           me_tm1637_set_brightness(disp_one, CONFIG_ME_DIFACE_DISP_BRIGHTNESS);
#ifdef CONFIG_ME_DIFACE_DISP_ONE_6SEGS
  #define ME_DIFACE_DISP_ONE_SEGS     TM1637_6SEGS_DM
  #define NUM_DIGITS                  6
#else
  #define ME_DIFACE_DISP_ONE_SEGS     TM1637_4SEGS
  #define NUM_DIGITS                  4
#endif
/* Caveat: num digits defined for the main display only */

typedef struct me_diface_state_s {
    enum {  STATE_IDLE,
            STATE_SELECT,
            STATE_VALUE_CHANGE,
            STATE_CONFIRM
        }                       state;
    enum {  CONFIRM_OK,
            CONFIRM_CANCEL,
            CONFIRM_SET,
            CONFIRM_ERR,
            CONFIRM_END
        }                       confirm;
    unsigned char               shifting;
    me_diface_item_t           *item;
    me_diface_item_t           *idle_item;
    me_diface_item_t           *second_item;

    TickType_t                  delay;
} me_diface_state_t;

static QueueHandle_t        queue;
static me_diface_state_t    ifstate = { 0 };
static me_tm1637_led_t     *disp_one;
static me_str_t             confirm_str[] = {
                                me_str("SET "),
                                me_str("CNCL"),
                                me_str("good"),
                                me_str("ERR "),
                                me_str("end ")
                            };

#ifdef CONFIG_ME_DIFACE_DEBUG
void state_debug()
{
    static char *state_str[] = {
        "   idle",
        " select",
        "  value",
        "confirm"
    };
    me_debug("DIFACE", "state: %s, delay: %d, "
                       "item: %.*s [% 4.4f], "
                       "nshift: %d, vshift: %d, "
                       "idle_item: %.*s [% 4.4f] "
                         , state_str[ifstate.state]
                         , ifstate.delay
                         , ifstate.item->name->len, ifstate.item->name->data
                         , ifstate.item->value
                         , ifstate.item->nshift, ifstate.item->vshift
                         , ifstate.idle_item->name->len, ifstate.idle_item->name->data
                         , ifstate.idle_item->value
                         );
}


void encoder_debug(me_ncoder_event_t *ev)
{
    static char *event_str[] = {
      "released", "released long"
    , "pressed", "pressed long"
    , "rotate left", "rotate right"
    };

    me_debug("DIFACE", "button: %d, event: %d (%s)"
                     , ev->button
                     , ev->state, event_str[ev->state]);
}
#else
#define state_debug()
#define encoder_debug(x)
#endif


void me_diface_display(me_tm1637_led_t *disp, u_char *p, int len, double val, int shift)
{
    //u_char     *p;
    uint32_t    u32;
    u_char      buf[10+1+21+1], is_neg;

    if( !p )
    {
        if( val < 0 )
        {
            is_neg  = 1;
            val    *= (-1);
        } else 
            is_neg = 0;

        u32 = (val - (uint32_t)val) * (FRAC_MULT);

        p = buf + 10+1+21;

        if( u32 )
        {
            do {
                *p-- = '0'+(u32 % 10);
                u32 /= 10;
            } while( u32 );

            *p-- = '.';
        }

        u32 = (uint32_t)val;
        if( u32 )
        {
            do {
                *p-- = '0'+(u32 % 10);
                u32 /= 10;
            } while( u32 );
        } else
            *p-- = '0';
        if( is_neg )
            *p = '-';
        else
            p++;

        len = buf + 10+1+21+1 - p;
    }

//me_debug("DIFACE", "NUM: %f, SFT: %d, STR(%d): '%.*s'", val, shift, len, len, p);
    if( len - shift < NUM_DIGITS )
    {
        shift = len - NUM_DIGITS;
        if( shift < 0 )
        {
            shift = 0;
        } else 
            len -= shift;
    } else
        len = NUM_DIGITS;
//me_debug("DIFACE", "SHIFTED NUM: %f, SFT: %d, STR(%d): '%.*s'", val, shift, len, len, p+shift);

    me_tm1637_set_text(disp, p + shift, len);
}


void do_shift(int change)
{
    ifstate.shifting = 1;

    if( ifstate.state == STATE_VALUE_CHANGE || ifstate.state == STATE_IDLE )
    {
        ifstate.item->vshift -= change;
        DISP_ONE_NUM(ifstate.item);
    } else 
    {
        ifstate.item->nshift -= change;
        DISP_ONE_ITEM(ifstate.item);
    }
}


void do_set()
{
    me_debug_assert(ifstate.item->set);

    ifstate.confirm = ( ifstate.item->set(ifstate.item) == ESP_OK ) 
                            ? CONFIRM_SET : CONFIRM_ERR;

    DISP_ONE_STR(&confirm_str[ifstate.confirm]);

    ifstate.delay = ME_DIFACE_REFRESH_DELAY;
    ifstate.state = STATE_IDLE;
    ifstate.item  = ifstate.idle_item;
}


void do_pos(int change)
{
    static double            add = 0.0;
    TickType_t               ticks, diff;
    static TickType_t        last_ticks = 0;
    me_diface_range_t       *range;

    switch( ifstate.state )
    {
        case STATE_IDLE:
            ifstate.state = STATE_SELECT;
            DISP_ONE_ITEM(ifstate.item);
        break;
        case STATE_SELECT:
            ifstate.item = (change > 0) ? ifstate.item->next 
                                        : ifstate.item->prev;
            
            if( ifstate.item->flags & ME_DIFACE_ITEM_FLAG_IDLE )
                ifstate.idle_item = ifstate.item;

            DISP_ONE_ITEM(ifstate.item);
        break;
        case STATE_VALUE_CHANGE:
            ticks = xTaskGetTickCount();
            range = ifstate.item->range;

            diff = ticks - last_ticks;
            if( diff < 20 )
                add *= range->stepk;
            else
                add  = range->step;

            last_ticks = ticks;

            me_debug("DIFACE", "diff: %d, add: %4.2f", diff, add);
            ifstate.item->value += add * change;
            if( ifstate.item->value > range->max )
                ifstate.item->value = range->min;
            else if( ifstate.item->value < range->min )
                ifstate.item->value = range->max;

            DISP_ONE_NUM(ifstate.item);
        break;
        case STATE_CONFIRM:
            ifstate.confirm = (ifstate.confirm == CONFIRM_CANCEL) 
                                ? CONFIRM_OK : CONFIRM_CANCEL;
            
            DISP_ONE_STR(&confirm_str[ifstate.confirm]);
        break;
    }
}


void do_push()
{
    switch( ifstate.state )
    {
        case STATE_IDLE:
            ifstate.state = STATE_SELECT;
            ifstate.delay = ME_DIFACE_SET_DELAY;

            DISP_ONE_ITEM(ifstate.item);
        break; 
        case STATE_SELECT:
            DISP_TWO_STR(ifstate.item->name);

            if( ifstate.item->items )
            {
                ifstate.item = ifstate.item->items;

                DISP_ONE_ITEM(ifstate.item);
            } else if( ifstate.item->flags & ME_DIFACE_ITEM_FLAG_SP )
            {
                ifstate.state = STATE_VALUE_CHANGE;

                DISP_ONE_NUM(ifstate.item);
            } else if( ifstate.item->flags & ME_DIFACE_ITEM_FLAG_CONFIRM )
            {
                ifstate.state   = STATE_CONFIRM;
                ifstate.confirm = CONFIRM_OK;

                DISP_ONE_STR(&confirm_str[ifstate.confirm]);
            } else if( ifstate.item->flags & ME_DIFACE_ITEM_FLAG_SET )
            {
                do_set();
            } else {
                ifstate.state = STATE_IDLE;
                ifstate.delay = ME_DIFACE_REFRESH_DELAY;
            }
        break;
        case STATE_VALUE_CHANGE:
            if( ifstate.item->flags & ME_DIFACE_ITEM_FLAG_CONFIRM )
            {
                ifstate.state   = STATE_CONFIRM;
                ifstate.confirm = CONFIRM_OK;

                DISP_ONE_STR(&confirm_str[ifstate.confirm]);
            } else 
            {
                do_set();
            }
        break;
        case STATE_CONFIRM:
            do_set();
        break;
    }
}


void me_diface_task(void *ign)
{
    me_ncoder_event_t   e[1];

    while(1)
    {
        if( xQueueReceive(queue, e, ifstate.delay) )
        {
            encoder_debug(e);

            switch( e->state )
            {
                case ME_NCODER_PRESSED:
                break;
                case ME_NCODER_RELEASED_LONG:
                    ifstate.shifting = 0;
                    DISP_ONE_RESET;
                    DISP_TWO_RESET;
                break;
                case ME_NCODER_PRESSED_LONG:
                    if( ifstate.shifting )
                        break;

                    ifstate.delay = ME_DIFACE_REFRESH_DELAY;

                    ifstate.item  = ifstate.idle_item;
                    ifstate.state = STATE_IDLE;

                    DISP_ONE_STR(ifstate.item->name);
                    DISP_TWO_STR(ifstate.second_item->name);
                break;
                case ME_NCODER_RELEASED:
                    if( !ifstate.shifting )
                        do_push();
                    ifstate.shifting = 0;
                break;
                case ME_NCODER_ROTATED_LEFT:
                    if( e->button )
                        do_shift(-1);
                    else 
                        do_pos(-1);
                break;
                case ME_NCODER_ROTATED_RIGHT:
                    if( e->button )
                        do_shift(+1);
                    else 
                        do_pos(+1);
                break;
            }
        } else if( ifstate.state != STATE_IDLE )
        {
            DISP_ONE_STR(&confirm_str[CONFIRM_END]);

            ifstate.item  = ifstate.idle_item;
            ifstate.state = STATE_IDLE;
            ifstate.delay = ME_DIFACE_REFRESH_DELAY;
            
            continue;
        }

        if( ifstate.state == STATE_IDLE )
        {
            DISP_ONE_NUM(ifstate.item);
            DISP_TWO_NUM(ifstate.second_item);
        }

        state_debug();
    }
}


esp_err_t me_diface_init(me_diface_item_t *root, me_diface_item_t *second)
{
    uint8_t         btn_pins[] = { CONFIG_ME_NCODER_PIN_BTN };
    esp_err_t       err;

    ifstate.item         = ifstate.idle_item = root;
    ifstate.second_item  = second;
    ifstate.state        = STATE_IDLE;
    ifstate.delay        = ME_DIFACE_REFRESH_DELAY;

    disp_one = me_tm1637_init(
                      CONFIG_ME_DIFACE_PIN_DISP_ONE_CLK
                    , CONFIG_ME_DIFACE_PIN_DISP_ONE_DTA
                    , ME_DIFACE_DISP_ONE_SEGS);
    me_debug_assert(disp_one);
#ifdef CONFIG_ME_DIFACE_HAS_SECOND_DISPLAY
    disp_two = me_tm1637_init(
                      CONFIG_ME_DIFACE_PIN_DISP_TWO_CLK
                    , CONFIG_ME_DIFACE_PIN_DISP_TWO_DTA
                    , ME_DIFACE_DISP_TWO_SEGS);
    me_debug_assert(disp_two);
#endif

    queue = NULL;
    err   = me_ncoder_init(btn_pins, 1
                , CONFIG_ME_NCODER_PIN_A, CONFIG_ME_NCODER_PIN_B, &queue);

    xTaskCreate(me_diface_task, "diface", 2048, NULL, 10, NULL);

    return err;
}
