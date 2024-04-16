/* My ESP CQ charger (BQ25895)
 * File: me_cq.c
 * Started: Fri Mar 29 09:56:34 MSK 2024
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2024 ..."
 */

#include <me_debug.h>
#include <esp_err.h>

#include <me_cq.h>

#define ME_CQ_STORE_NAME
#ifdef CONFIG_ME_DEBUG
static const char   *TAG    = "CQ";
#define dd(...)     ESP_LOGI(TAG, __VA_ARGS__)
#define de(...)     ESP_LOGE(TAG, __VA_ARGS__)
#else
#define dd(...)
#define de(...)
#endif


me_cq_batt_info_t me_cq_batt_info = {
    .level_max       = 4100,
    .level_low       = 3400,
    .level_extra_low = 3280,
};

me_cq_t   me_cq_batt = {
    .mv     = 4100,
    .level  = 0,
    .status = 0,
};


esp_err_t me_cq_ctrl(uint8_t reg, uint8_t mask, uint8_t val)/*{{{*/
{
    esp_err_t       err;
    uint8_t         res, cmd;

    err = me_i2c_req_resp(ME_CQ_I2C_NUM, ME_CQ_I2C_ADDR, &reg, 1, &res, 1);
    if( err != ESP_OK )
    {
        de("Cannot read reg 0x%02X. Err code %d (%s)", reg, err, esp_err_to_name(err));
        return ESP_FAIL;
    }

    if( (res & mask) == val )
    {
        dd(" -- Value change nothing."
            " reg value: 0x%02X,"
            " mask: 0x%02X,"
            " value: 0x%02X", res, mask, val);
        return ESP_OK;
    } 

    cmd = (res & ~mask) | val;
    dd("CQ set reg 0x%02X. Read: 0x%02X, Write: 0x%02X, val: 0x%02X", reg, res, cmd, val);
    err = me_i2c_write_reg(ME_CQ_I2C_NUM, ME_CQ_I2C_ADDR, reg, cmd);
    if( err != ESP_OK )
    {
        dd("Cannot write 0x%02X with value 0x%02X. Err code %d (%s)"
            , reg, cmd, err, esp_err_to_name(err));
        return ESP_FAIL;
    }

    return ESP_OK;
}/*}}}*/


esp_err_t me_cq_read()/*{{{*/
{
    esp_err_t   err;
    uint8_t     res[4], reg, u8;
    int16_t     i16;

    reg = 0x0B;
    err = me_i2c_req_resp(ME_CQ_I2C_NUM, ME_CQ_I2C_ADDR, &reg, 1, res, 4);
    if( err != ESP_OK )
    {
        de("Cannot read status registers. Err code %d (%s)", err, esp_err_to_name(err));
        me_cq_batt.level  = 0;

        return ESP_FAIL;
    }

    me_cq_batt.charging = res[0] & (ME_CQ_MASK_CHRG_STAT | ME_CQ_MASK_PG_STAT);

    u8 = (res[1] & (ME_CQ_MASK_BOOST_FAULT | 
                      ME_CQ_MASK_CHRG_FAULT | 
                      ME_CQ_MASK_BATT_FAULT));


    me_cq_batt.status = u8 | (res[3] & ME_CQ_MASK_THERM_STAT);

    me_cq_batt.mv = 2304+(res[3] & ME_CQ_MASK_BATV) * 20;

    if( me_cq_batt.status )
    {
        // charging 
        switch( me_cq_batt.status & ME_BATT_STS_CHRG_MASK )
        {
            case ME_BATT_STS_CHRG_NO:
                me_cq_batt.level = 0;
            break;
            case ME_BATT_STS_CHRG_PRE:
                me_cq_batt.level = 30;
            break;
            case ME_BATT_STS_CHRG_FAST:
                me_cq_batt.level = 60;
            break;
            case ME_BATT_STS_CHRG_DONE:
                me_cq_batt.level = 100;
            break;
        }
    } else {
        // should never be negative
        i16 = 100 * (me_cq_batt.mv - me_cq_batt_info.level_extra_low) / 
                    (me_cq_batt_info.level_max - me_cq_batt_info.level_extra_low);
        if( i16 > 100 )
            i16 = 100;
        me_cq_batt.level = i16;
    } 
    dd("Batt. V: %d mV, lvl: %d, T: %d, Chg: 0x%02X, Stu: 0x%02X"
        , me_cq_batt.mv, me_cq_batt.level, me_cq_batt.temp, me_cq_batt.charging, me_cq_batt.status);

    return ESP_OK;
}/*}}}*/


esp_err_t me_cq_init()
{
    esp_err_t   err;

    /* TODO: Set Current limit */
    err = me_i2c_ping(ME_CQ_I2C_NUM, ME_CQ_I2C_ADDR);

    dd("CQ charger ping by I2C at address 0x%02X. Return %d (%s)"
            , ME_CQ_I2C_ADDR, err, esp_err_to_name(err));

    return err;
}
