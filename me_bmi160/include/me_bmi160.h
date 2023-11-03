/* My ESP BMI160 gyroscope
 * File: me_bmi160.h
 * Started: Fri Nov  3 11:29:16 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 */

#ifndef _ME_BMI160_H
#define _ME_BMI160_H

#define ME_BMI160_I2C_ADDR                     0x68

#define ME_BMI160_CMD_SOFT_RESET               0xb6
#define ME_BMI160_CMD_ACCEL_PM_NORM            0x11
#define ME_BMI160_CMD_ACCEL_PM_LOW             0x12

#define ME_BMI160_REG_CHIP_ID                  0x00 
#define ME_BMI160_REG_PMU_STATUS               0x03
#define ME_BMI160_REG_DATA                     0x04
#define ME_BMI160_REG_DATA_0                   0x04
#define ME_BMI160_REG_DATA_END                 0x0F
#define ME_BMI160_REG_STATUS                   0x1B
#define ME_BMI160_REG_INT_STATUS               0x1C
#define ME_BMI160_REG_INT_STATUS_0             0x1C
#define ME_BMI160_REG_INT_STATUS_1             0x1D
#define ME_BMI160_REG_INT_STATUS_2             0x1E
#define ME_BMI160_REG_INT_STATUS_3             0x1F
#define ME_BMI160_REG_ACCEL_CONF               0x40
#define ME_BMI160_REG_ACCEL_RANGE              0x41
#define ME_BMI160_REG_GYRO_CONF                0x42
#define ME_BMI160_REG_GYRO_RANGE               0x43
#define ME_BMI160_REG_INT_EN                   0x50
#define ME_BMI160_REG_INT_EN_0                 0x50
#define ME_BMI160_REG_INT_EN_1                 0x51
#define ME_BMI160_REG_INT_EN_2                 0x52
#define ME_BMI160_REG_INT_OUT_CTRL             0x53
#define ME_BMI160_REG_INT_LATCH                0x54
#define ME_BMI160_REG_INT_MAP                  0x55
#define ME_BMI160_REG_INT_MAP_0                0x55
#define ME_BMI160_REG_INT_MAP_1                0x56
#define ME_BMI160_REG_INT_MAP_2                0x57
#define ME_BMI160_REG_INT_DATA                 0x58
#define ME_BMI160_REG_INT_DATA_0               0x58
#define ME_BMI160_REG_INT_DATA_1               0x59
#define ME_BMI160_REG_INT_TAP                  0x63
#define ME_BMI160_REG_INT_TAP_0                0x63
#define ME_BMI160_REG_INT_TAP_1                0x64
#define ME_BMI160_REG_INT_FLAT                 0x67
#define ME_BMI160_REG_INT_FLAT_0               0x67
#define ME_BMI160_REG_INT_FLAT_1               0x68
#define ME_BMI160_REG_CMD                      0x7e


#endif
