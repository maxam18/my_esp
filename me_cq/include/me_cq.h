/* My ESP CQ charger (BQ25895)
 * File: me_cq.h
 * Started: Fri Mar 29 09:56:40 MSK 2024
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2024 ..."
 */

#ifndef _ME_CQ_H
#define _ME_CQ_H

#include <stdint.h>
#include <me_i2c.h>

#define ME_CQ_LEVEL_MV_MAX              4100
#define ME_CQ_LEVEL_MV_LOW              3400
#define ME_CQ_LEVEL_MV_EXTRA_LOW        3250
#define ME_CQ_I2C_NUM                   I2C_NUM_0
#define ME_CQ_I2C_ADDR                  0x6A

#define ME_BATT_STS_EXT_PWR             0x04
#define ME_BATT_STS_CHRG_MASK           0x18
#define ME_BATT_STS_CHRG_NO             0x00
#define ME_BATT_STS_CHRG_PRE            0x08
#define ME_BATT_STS_CHRG_FAST           0x10
#define ME_BATT_STS_CHRG_DONE           0x18
#define ME_BATT_ERR_CHIP_THERM          0x80
#define ME_BATT_ERR_BOOST_FAULT         0x40
#define ME_BATT_ERR_CHRG_FAULT          0x30
#define ME_BATT_ERR_BATT_FAULT          0x08
#define ME_BATT_ERR_BATT_THIGH          0x02 // temperature high
#define ME_BATT_ERR_BATT_TLOW           0x01 // temperature low

#define ME_CQ_WD_REG                    0x07
#define ME_CQ_WD_MASK                   0x30
#define ME_CQ_WD_CMD_OFF                0x00
#define ME_CQ_WD_CMD_DEF                0x20
#define ME_CQ_OTG_CFG_REG               0x03
#define ME_CQ_OTG_CFG_MASK              0x20
#define ME_CQ_OTG_CFG_OFF               0x00
#define ME_CQ_OTG_CFG_ON                0x20
#define ME_CQ_VREG_REG                  0x06
#define ME_CQ_VREG_MASK                 0xFC
#define ME_CQ_VREG_CMD_4208             0x5C
#define ME_CQ_TREG_REG                  0x08
#define ME_CQ_TREG_MASK                 0x03
#define ME_CQ_TREG_CMD_80               0x01
#define ME_CQ_TREG_CMD_100              0x03
#define ME_CQ_TREG_CMD_120              0x03
#define ME_CQ_ADCCTRL_REG               0x02
#define ME_CQ_ADCCTRL_MASK              0xC0
#define ME_CQ_ADCCTRL_CMD_CONT          0xC0
#define ME_CQ_ADCCTRL_CMD_ONESHOT       0x80
#define ME_CQ_MASK_THERM_STAT           0x80 // REG 0E - 1 bit  (7)
#define ME_CQ_MASK_BOOST_FAULT          0x40 // REG 0C - 1 bit  (6)
#define ME_CQ_MASK_CHRG_FAULT           0x30 // REG 0C - 2 bits (4-5)
#define ME_CQ_MASK_BATT_FAULT           0x08 // REG 0C - 1 bit  (3)
#define ME_CQ_MASK_CHRG_STAT            0x18 // REG 0B - 2 bits (3-4)
#define ME_CQ_MASK_PG_STAT              0x04 // REG 0B - 1 bit  (2)
#define ME_CQ_MASK_BATV                 0x7F // REG 0E - 6 bits

typedef struct {
    int16_t     level_max;
    int16_t     level_low;
    int16_t     level_extra_low;
    int8_t      spare[14];          // pp_batt compatibility
}  me_cq_batt_info_t;

typedef struct {
    int16_t     mv;         /*!< battery voltage */
    int16_t     temp;       /*!< raw value for now*/
    uint8_t     level;      /*!< percentage */
    uint8_t     charging;   /*!< not 0 if charging */
    uint8_t     status;     /*!< batt error */
}  me_cq_t;

extern me_cq_t me_cq_batt;

#define me_cq_5v_on()           me_cq_ctrl(ME_CQ_OTG_CFG_REG, ME_CQ_OTG_CFG_MASK, ME_CQ_OTG_CFG_ON)
#define me_cq_5v_off()          me_cq_ctrl(ME_CQ_OTG_CFG_REG, ME_CQ_OTG_CFG_MASK, ME_CQ_OTG_CFG_OFF)
#define me_cq_wd_off()          me_cq_ctrl(ME_CQ_WD_REG, ME_CQ_WD_MASK, ME_CQ_WD_CMD_OFF)
#define me_cq_wd_def()          me_cq_ctrl(ME_CQ_WD_REG, ME_CQ_WD_MASK, ME_CQ_WD_CMD_DEF)
#define me_cq_vreg_4208()       me_cq_ctrl(ME_CQ_VREG_REG, ME_CQ_VREG_MASK, ME_CQ_VREG_CMD_4208)
#define me_cq_treg_100()        me_cq_ctrl(ME_CQ_TREG_REG, ME_CQ_TREG_MASK, ME_CQ_TREG_CMD_100)
#define me_cq_adc_cont()        me_cq_ctrl(ME_CQ_ADCCTRL_REG, ME_CQ_ADCCTRL_MASK, ME_CQ_ADCCTRL_CMD_CONT)
#define me_cq_adc_oneshoot()    me_cq_ctrl(ME_CQ_ADCCTRL_REG, ME_CQ_ADCCTRL_MASK, ME_CQ_ADCCTRL_CMD_ONESHOT)

esp_err_t me_cq_ctrl(uint8_t reg, uint8_t mask, uint8_t val);
esp_err_t me_cq_init();
esp_err_t me_cq_read();

esp_err_t me_cq_save(me_cq_batt_info_t *info);

#endif
