/* My ESP Luminox sensor reader
 * File: me_lox.c
 * Started: Thu Apr  7 16:07:58 MSK 2022
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2022 ..."
 */

#include "esp_system.h"
#include "esp_log.h"

#include "driver/uart.h"

#include <me_debug.h>

#include <me_lox.h>

#define LOX_ROW_LEN (sizeof("O 0198.4 T +25.2 P 0991 % 020.02 e 0000\r\n")-1)


esp_err_t me_lox_read(int uart_num, me_lox_t *r)
{
    int         n;
    uint8_t     data[128], *p, i, ch;
    enum        { STATE_S, STATE_R} state = STATE_S;
    uint16_t    val = 0;
    uint16_t   *vp[] = { &r->ppressure, &r->temperature, &r->pressure
                        , &r->value, &r->error };

    uart_flush_input(uart_num);

    i = 8; /* 4 seconds maximum delay */ 
    do {
        vTaskDelay(pdMS_TO_TICKS(500));
        uart_get_buffered_data_len(uart_num, (size_t *)&n);
        if( i-- == 0 )
            return ESP_FAIL;
    } while( n < LOX_ROW_LEN * 2 );

    n = uart_read_bytes(uart_num, data, sizeof(data), 0);

    if( n < LOX_ROW_LEN )
        return ESP_FAIL;

    p = data;
    while( *p != 'O' && n ) { p++; n--; }

    if( *(p + LOX_ROW_LEN - 1) != '\n' )
        return ESP_FAIL;

    i = 0;
    while( n-- )
    {
        ch = *p++ - '0';
        switch( state )
        {
            case STATE_S:
                if( ch < 10 ) {
                    state = STATE_R;
                    val   = ch;
                }
                break;
            case STATE_R:
                if( ch < 10 ) {
                    val = val*10 + ch;
                } else if ( ch != (uint8_t)('.' - '0') ) {
                    *vp[i++] = val;
                    if( i == 5 )
                        n = 0;
                    state = STATE_S;
                }
                break;
        }
    }
    
    return ( i == 5 ) ? ESP_OK : ESP_FAIL;
}


esp_err_t me_lox_init(int uart_num, int tx, int rx)
{
    uart_config_t   uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, tx, rx
            , UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    return uart_driver_install(uart_num, 256, 0, 0, NULL, 0);
}