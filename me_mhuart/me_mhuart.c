/* MH-Z19 CO2 sensor UART mode
 * File: mh_z19.c
 * Started: Wed 27 Nov 2019 02:26:03 PM MSK
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2019 ..."
 */
#include <stdint.h>
#include "esp_system.h"
#include "esp_err.h"
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


static esp_err_t send_req(int uart_num, uint8_t req, uint8_t *reqval)
{
    int         n;
    uint8_t     cmd[] = { 0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86 };

    uart_flush_input(uart_num);

    cmd[2] = req;
    if( reqval )
    {
        for(n = 3; n < 8; n++)
            cmd[n] = *reqval++;
    }

    cmd[8] = checksum(cmd);

    me_debug( "MHUART", "Send: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X"
            , (unsigned int)cmd[0], (unsigned int)cmd[1], (unsigned int)cmd[2], (unsigned int)cmd[3]
            , (unsigned int)cmd[4], (unsigned int)cmd[5], (unsigned int)cmd[6], (unsigned int)cmd[7], (unsigned int)cmd[8]);

    n = uart_write_bytes(uart_num, (const char *) cmd, 9);

    if( n != 9 )
    {
        ESP_LOGE(TAG, "Send 9 but wrote %d", n);
        return ESP_FAIL;
    }

    return ESP_OK;
}


static esp_err_t read_resp(int uart_num, uint8_t *data)
{
    int    n;

    n = uart_read_bytes(uart_num, data, MH_UART_BUF_SIZE, pdMS_TO_TICKS(60));

    me_debug( "MHUART", "Recv: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X"
            , (unsigned int)data[0], (unsigned int)data[1], (unsigned int)data[2], (unsigned int)data[3]
            , (unsigned int)data[4], (unsigned int)data[5], (unsigned int)data[6], (unsigned int)data[7], (unsigned int)data[8]);

    if( n != 9 )
    {
        ESP_LOGE(TAG, "Expect 9 but read %d", n);
        return ESP_FAIL;
    }

    if( data[8] != checksum(data) )
    {
        ESP_LOGW(TAG, "CRC incorrect. 0x%02X 0x%02X 0x%02X ... 0x%02X"
                    , data[0], data[1], data[2], data[8]);
        return ESP_FAIL;
    }

    return ESP_OK;
}


int me_mhuart_read_concentration(int uart_num, uint8_t cmd)
{
    int             n;
    unsigned char   data[MH_UART_BUF_SIZE];

    if( send_req(uart_num, cmd, NULL) )
        return -1;

    if( read_resp(uart_num, data) )
        return -1;

    if( data[1] != cmd )
    {
        ESP_LOGE(TAG, "Command rejected. Send %X recv %X", cmd, data[1]);
        return ESP_FAIL;
    }

    n = (data[2] << 8) | data[3];
    if( cmd == MH_CMD_READ_LONG )
        n = (n<<16) + (data[4] << 8) + data[5];

    return n;
}


esp_err_t me_mhuart_auto_calibration(int uart_num, int is_on)
{
    uint8_t  val[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };

    if( is_on )
        val[0] = 0xa0;

    return send_req(uart_num, MH_CMD_CALIBRATE_AUTO, val);
}


esp_err_t me_mhuart_calibrate(int uart_num, int ppm)
{
    uint8_t  val[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t  cmd = MH_CMD_CALIBRATE_ZERO;

    me_debug( "MHUART", "MH calibration requested for %d ppm", ppm);

    if( ppm )
    {
        val[0] = (ppm >> 8) & 0xFF;
        val[1] = ppm & 0xFF;
        cmd = MH_CMD_CALIBRATE_SPAN;
    }

    return send_req(uart_num, cmd, val);
}


esp_err_t me_mhuart_set_mode(int uart_num, uint8_t mode)
{
    uint8_t  val[5] = { mode, 0x00, 0x00, 0x00, 0x00 };

    return send_req(uart_num, MH_CMD_SET_MODE, val);
}
