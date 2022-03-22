/* MH-Z19 CO2 sensor UART mode
 * File: mh_z19.c
 * Started: Wed 27 Nov 2019 02:26:03 PM MSK
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2019 ..."
 */

#include "esp_system.h"
#include "esp_log.h"

#include "driver/uart.h"

#include <me_mhuart.h>
#include <me_debug.h>

/* TODO: should wait before read after the calibration */

#define MH_CMD_CALIBRATE_AUTO   0x79
#define MH_CMD_CALIBRATE_ZERO   0x87
#define MH_CMD_CALIBRATE_SPAN   0x88
#define MH_CMD_SET_MODE         0x78

static const char *TAG = "MH";

static uint8_t checksum(uint8_t *data)
{
    uint8_t i, res = 0;

    for( i = 1; i < 8; i++)
        res += data[i];

    return (0xFF - res) + 1;
}

static u_char *readwrite(int uart_num, uint8_t command, uint8_t *val, u_char *buf)
{
    int         n;
    uint8_t     cmd[] = { 0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86 };
    uint8_t     *data, sbuf[MH_UART_BUF_SIZE];

    data = buf ? buf : sbuf;

    uart_flush_input(uart_num);

    cmd[2] = command;
    if( val )
    {
        for(n = 3; n < 8; n++)
            cmd[n] = *val++;
    }

    cmd[8] = checksum(cmd);

    me_debug( "MHUART-RW", "MH send: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X"
            , (u_int)cmd[0], (u_int)cmd[1], (u_int)cmd[2], (u_int)cmd[3]
            , (u_int)cmd[4], (u_int)cmd[5], (u_int)cmd[6], (u_int)cmd[7], (u_int)cmd[8]);

    n = uart_write_bytes(uart_num, (const char *) cmd, 9);

    if( n != 9 )
    {
        ESP_LOGE(TAG, "Send 9 but wrote %d", n);
        return NULL;
    }

    n = uart_read_bytes(uart_num, data, MH_UART_BUF_SIZE, 60/portTICK_RATE_MS);

    me_debug( "MHUART-RW", "MH recv: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X"
            , (u_int)data[0], (u_int)data[1], (u_int)data[2], (u_int)data[3]
            , (u_int)data[4], (u_int)data[5], (u_int)data[6], (u_int)data[7], (u_int)data[8]);

    if( n != 9 )
    {
        ESP_LOGE(TAG, "Expect 9 but read %d", n);
        return NULL;
    }

    if( data[8] != checksum(data) )
    {
        ESP_LOGW(TAG, "CRC incorrect. 0x%02X 0x%02X 0x%02X ... 0x%02X"
                    , data[0], data[1], data[2], data[8]);
        return NULL;
    }

    if( data[1] != command )
    {
        ESP_LOGE(TAG, "Command rejected. Send %X recv %X", command, data[1]);
        return NULL;
    }

    return data;
}


int me_mhuart_read_concentration(int uart_num, uint8_t cmd)
{
    int     n;
    u_char  data[MH_UART_BUF_SIZE];

    if( !readwrite(uart_num, cmd, NULL, data) )
        return -1;

    n = (data[2] << 8) | data[3];
    if( cmd == MH_CMD_READ_LONG )
        n = (n<<16) + (data[4] << 8) + data[5];

    return n;
}


int me_mhuart_auto_calibration(int uart_num, int is_on)
{
    uint8_t  val[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };

    if( is_on )
        val[0] = 0xa0;

    return readwrite(uart_num, MH_CMD_CALIBRATE_AUTO, val, NULL) ? -1 : 0;
}

int me_mhuart_calibrate(int uart_num, int ppm)
{
    uint8_t  val[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
    u_char  *retval;

    me_debug( "MHUART-CALIB", "MH calibration requested for %d ppm", ppm);

    if( ppm )
    {
        val[0] = (ppm >> 8) & 0xFF;
        val[1] = ppm & 0xFF;
        retval = readwrite(uart_num, MH_CMD_CALIBRATE_SPAN, val, NULL);
    } else
        retval = readwrite(uart_num, MH_CMD_CALIBRATE_ZERO, NULL, NULL);

    return retval ? -1 : 0;
}

int me_mhuart_set_mode(int uart_num, uint8_t mode)
{
    uint8_t  val[5] = { mode, 0x00, 0x00, 0x00, 0x00 };

    return readwrite(uart_num, MH_CMD_SET_MODE, val, NULL) ? -1 : 0;
}



