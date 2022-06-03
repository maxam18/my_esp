/* MH741A Winsen gas concentration sensor
 * File: me_mh741.c
 * Started: Mon Jun 14 20:44:07 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */

#include <me_i2c.h>
#include <me_mh741.h>
#include <me_debug.h>

#define MH741_ADDR      0x55

static enum { MH741_UNKN, MH741_CONC } last_cmd[2] = { MH741_UNKN, MH741_UNKN };

static uint8_t checksum(uint8_t *data)
{
    uint8_t i, res = 0;

    for( i = 0; i < 9; i++)
        res += data[i];

    return (0xFF - res) + 1;
}


esp_err_t me_mh741_read(i2c_port_t port, int16_t *value)
{
    esp_err_t           err;
    uint8_t             req[] = {  0x96, 0x00, 0x00, 0x00, 0x00 
                                 , 0x00, 0x00, 0x00, 0x00, 0x6A };
    uint8_t             resp[10];
    static int8_t       cnt = 5;

    if( last_cmd[port] == MH741_CONC && cnt-- )
    {
        err = me_i2c_read(port, MH741_ADDR, resp, 10);
        cnt = 5;
    }
    else
    {
        err = me_i2c_req_resp(port, MH741_ADDR, req, 10, resp, 10);
        me_debug( "mh741", "Sent: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X"
                                " 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X"
                                ", err: %s"
            , req[0], req[1], req[2], req[3], req[4]
            , req[5], req[6], req[7], req[8], req[9]
            , esp_err_to_name(err));
    }

    if( err != ESP_OK )
    {
        last_cmd[port] = MH741_UNKN;
        return err;
    }

    if( resp[9] == checksum(resp) )
    {
        me_debug( "mh741", "Read: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X"
                                " 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X"
            , resp[0], resp[1], resp[2], resp[3], resp[4]
            , resp[5], resp[6], resp[7], resp[8], resp[9]);

        *value = (resp[5] << 8) | resp[6];
    } else
    {
        me_debug( "mh741", "Checksum incorrect. 0x%02X != 0x%02X (received)"
                         , checksum(resp), resp[9]);

        last_cmd[port] = MH741_UNKN;
        *value   = -1;
        
        err = ESP_FAIL;
    }

    return err;
}


esp_err_t me_mh741_calibrate(i2c_port_t port, int ppm)
{
    esp_err_t   err;
    uint8_t     req[] = {  0xA0, 0x00, 0x00, 0x00, 0x00 
                         , 0x00, 0x00, 0x00, 0x00, 0x60 };

    if( ppm )
    { /* span */
        req[0] = 0xAA;
        req[1] = (ppm >> 8) & 0xFF;
        req[2] = ppm & 0xFF;
        req[9] = checksum(req);
    }

    last_cmd[port] = MH741_UNKN;

    err = me_i2c_write(port, MH741_ADDR, req, 10);
    me_debug( "mh741", "Sent: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X"
                            " 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X"
                            ", err: %s"
        , req[0], req[1], req[2], req[3], req[4]
        , req[5], req[6], req[7], req[8], req[9]
        , esp_err_to_name(err));

    if( err != ESP_OK )
    {
        me_debug( "mh741", "Calibration write error %d: %s"
                         , err, esp_err_to_name(err));
    }

    return err;
}
