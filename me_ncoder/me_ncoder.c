/* My ESP multi encoder
 * File: me_ncoder.c
 * Started: Tue Mar 22 11:59:02 MSK 2022
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2022 ..."
 */

#include <me_debug.h>
#include "me_ncoder.h"
#include <freertos/semphr.h>

#define INTERVAL_US     2000
#define WAIT_MAX_US     INTERVAL_US * 10
#define LONG_PRESS_US   500000
#define PRESS_LEVEL     0

typedef struct {
    uint8_t    *pins;
    uint8_t     len;

    uint8_t     pina;
    uint8_t     pinb;
    
    uint8_t     btn;

    uint8_t     path;
    
    uint64_t    wait_us;

    me_ncoder_state_t   state;
} me_ncoder_t;

me_ncoder_t                 enc[1];
static QueueHandle_t        queue;
static SemaphoreHandle_t    mutex;

/* valid transitions 1, 2, 4, 7, 8, 11, 13, 14*/
#define VALID_TRANSITION 0x6996
#define PATH_CW          0x4B /* 01b => 00b => 10b => 11b */
#define PATH_CCW         0x87 /* 10b => 00b => 01b => 11b */

static void run_encoder()
{
    uint8_t           u8;
    me_ncoder_event_t ev = {0,0};

    if( enc->state == ME_NCODER_PRESSED && enc->wait_us < WAIT_MAX_US )
    {
        enc->wait_us += INTERVAL_US;
        return;
    }

    u8 = enc->len;
    while( u8-- )
        ev.button |= ((gpio_get_level(enc->pins[u8]) == PRESS_LEVEL) << u8);

    if( ev.button )
    {
        if( enc->state == ME_NCODER_RELEASED )
        {
            enc->wait_us = 0;
            enc->state   = ME_NCODER_PRESSED;
            enc->btn     = (ev.button << 2 ) |
                            ((gpio_get_level(enc->pina) == 0)<<1) | 
                            (gpio_get_level(enc->pinb) == 0);
            ev.state     = ME_NCODER_PRESSED;
            goto send_queue;
        }

        enc->wait_us += INTERVAL_US;

        if( enc->state == ME_NCODER_PRESSED && enc->wait_us > LONG_PRESS_US )
        {
            ev.state = enc->state = ME_NCODER_PRESSED_LONG;
            goto send_queue;
        }
    } else if( enc->state != ME_NCODER_RELEASED )
    {
        enc->state = ME_NCODER_RELEASED;
        ev.state   = enc->wait_us > LONG_PRESS_US ? 
                            ME_NCODER_RELEASED_LONG : ME_NCODER_RELEASED;
        goto send_queue;
    }

    u8 = 0x0F & ((enc->path << 2) 
        | (gpio_get_level(enc->pinb) << 1ULL) 
        | gpio_get_level(enc->pina));

    if( ((1ULL << u8 ) & VALID_TRANSITION) == 0 )
        return;

    enc->path = (enc->path << 2) | u8;
    if( enc->path == PATH_CW )
        ev.state = ME_NCODER_ROTATED_RIGHT;
    else if ( enc->path == PATH_CCW )
        ev.state = ME_NCODER_ROTATED_LEFT;
    else
        return;

send_queue:
    ev.button = enc->btn;
    xQueueSendToBack(queue, &ev, 0);
}

static void call_encoder()
{
    if( !xSemaphoreTake(mutex, 0) )
        return;

    run_encoder();

    xSemaphoreGive(mutex);
}

esp_err_t me_ncoder_init(uint8_t *pins, uint8_t len, uint8_t pina, uint8_t pinb, xQueueHandle *qptr)
{
    uint8_t i;
    gpio_config_t   io_conf = {
        .mode       = GPIO_MODE_INPUT,
        .intr_type  = GPIO_INTR_DISABLE,
        .pull_down_en = 0,
        .pull_up_en = 0,
        .pin_bit_mask = (1ULL<<pina) | (1ULL<<pinb)
    };
    esp_timer_create_args_t timer_args = {
        .name = "me_ncoder",
        .arg = NULL,
        .callback = call_encoder,
        .dispatch_method = ESP_TIMER_TASK
    };
    esp_timer_handle_t timer;

    if( *qptr == NULL )
    {
        queue = xQueueCreate(10, sizeof(me_ncoder_event_t));
        *qptr = queue;
    } else 
        queue = *qptr;

    if( queue == NULL )
        return ESP_ERR_NO_MEM;
    
    mutex = xSemaphoreCreateMutex();
    if( mutex == NULL )
        return ESP_ERR_INVALID_ARG;

    if( len == 0 )
        return ESP_ERR_INVALID_ARG;

    enc->pins = pins;
    enc->len  = len;
    enc->pina = pina;
    enc->pinb = pinb;
    enc->path = 0;

    while( len-- )
        io_conf.pin_bit_mask |= 1ULL << *pins++;

    io_conf.pin_bit_mask |= (1ULL << pina) | (1ULL << pinb);

    if( gpio_config(&io_conf) != ESP_OK )
        return ESP_ERR_INVALID_ARG;

#ifdef CONFIG_ME_DEBUG
    i = 0;
    for( i = 0; i < enc->len; ++i ) 
    {
        me_debug("NCODER", "Button pin[%d] %d level %d", i, enc->pins[i]
                , gpio_get_level(enc->pins[i]));
    }
    me_debug("NCODER", "Button pina %d level %d", pina
            , gpio_get_level(pina));
    me_debug("NCODER", "Button pinb %d level %d", pinb
            , gpio_get_level(pinb));
#endif

    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, INTERVAL_US));

    return ESP_OK;
}
