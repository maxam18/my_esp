/* ADS111x ADC definitions
 * File: me_ads111x_defs.h
 * Started: Tue Jun 15 12:28:30 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */

#ifndef _ME_ADS111X_DEFS_H
#define _ME_ADS111X_DEFS_H

/**
 * @brief configuration definitions
 */
#define ME_ADS111x_CONF_OS_SNGL          0x8000 /* 0b1000 0000 0000 0000 */
#define ME_ADS111x_CONF_OS_CONT          0x0000 /* 0b0000 0000 0000 0000 */
#define ME_ADS111x_CONF_OS_MASK          0x7FFF /* 0b0111 1111 1111 1111 */

#define ME_ADS111x_CONF_MUX_A0_1         0x0000 /* 0b0000 0000 0000 0000 */
#define ME_ADS111x_CONF_MUX_A0_3         0x1000 /* 0b0001 0000 0000 0000 */
#define ME_ADS111x_CONF_MUX_A1_3         0x2000 /* 0b0010 0000 0000 0000 */
#define ME_ADS111x_CONF_MUX_A2_3         0x3000 /* 0b0011 0000 0000 0000 */
#define ME_ADS111x_CONF_MUX_A0_G         0x4000 /* 0b0100 0000 0000 0000 */
#define ME_ADS111x_CONF_MUX_A1_G         0x5000 /* 0b0101 0000 0000 0000 */
#define ME_ADS111x_CONF_MUX_A2_G         0x6000 /* 0b0110 0000 0000 0000 */
#define ME_ADS111x_CONF_MUX_A3_G         0x7000 /* 0b0111 0000 0000 0000 */
#define ME_ADS111x_CONF_MUX_MASK         0x8FFF /* 0b1000 1111 1111 1111 */

#define ME_ADS111x_CONF_PGA_MASK         0xF1FF /* 0b1111 0001 1111 1111 */
#define ME_ADS111x_CONF_PGA_6144         0x0000 /* 0b0000 0000 0000 0000 */
#define ME_ADS111x_CONF_PGA_4096         0x0200 /* 0b0000 0010 0000 0000 */
#define ME_ADS111x_CONF_PGA_2048         0x0400 /* 0b0000 0100 0000 0000 */
#define ME_ADS111x_CONF_PGA_1024         0x0600 /* 0b0000 0110 0000 0000 */
#define ME_ADS111x_CONF_PGA_0512         0x8000 /* 0b0000 1000 0000 0000 */
#define ME_ADS111x_CONF_PGA_0256         0xC000 /* 0b0000 1010 0000 0000 */

#define ME_ADS111x_CONF_MODE_MASK        0xFEFF /* 0b1111 1110 1111 1111 */
#define ME_ADS111x_CONF_MODE_CONT        0x0000 /* 0b0000 0000 0000 0000 */
#define ME_ADS111x_CONF_MODE_SNGL        0x0100 /* 0b0000 0001 0000 0000 */

#define ME_ADS111x_CONF_RATE_MASK        0xFF1F /* 0b1111 1111 0001 1111 */
#define ME_ADS111x_CONF_RATE_8           0x0000 /* 0b0000 0000 0000 0000 */
#define ME_ADS111x_CONF_RATE_16          0x0020 /* 0b0000 0000 0010 0000 */
#define ME_ADS111x_CONF_RATE_32          0x0040 /* 0b0000 0000 0100 0000 */
#define ME_ADS111x_CONF_RATE_64          0x0060 /* 0b0000 0000 0110 0000 */
#define ME_ADS111x_CONF_RATE_128         0x0080 /* 0b0000 0000 1000 0000 */
#define ME_ADS111x_CONF_RATE_250         0x00A0 /* 0b0000 0000 1010 0000 */
#define ME_ADS111x_CONF_RATE_475         0x00C0 /* 0b0000 0000 1100 0000 */
#define ME_ADS111x_CONF_RATE_860         0x00E0 /* 0b0000 0000 1110 0000 */

#define ME_ADS111x_CONF_COMP_MODE_MASK   0xFF7F /* 0b1111 1111 1110 1111 */
#define ME_ADS111x_CONF_COMP_MODE_DEF    0x0000 /* 0b0000 0000 0000 0000 */
#define ME_ADS111x_CONF_COMP_MODE_WIN    0x0000 /* 0b0000 0000 0001 0000 */

#define ME_ADS111x_CONF_COMP_POL_MASK    0xFFF7 /* 0b1111 1111 1111 0111 */
#define ME_ADS111x_CONF_COMP_POL_LOW     0x0000 /* 0b0000 0000 0000 0000 */
#define ME_ADS111x_CONF_COMP_POL_HIGH    0x0080 /* 0b0000 0000 0000 1000 */

#define ME_ADS111x_CONF_COMP_LAT_MASK    0x000B /* 0b1111 1111 1111 1011 */
#define ME_ADS111x_CONF_COMP_LAT_NON     0x0000 /* 0b0000 0000 0000 0000 */
#define ME_ADS111x_CONF_COMP_LAT_LAT     0x0004 /* 0b0000 0000 0000 0100 */

#define ME_ADS111x_CONF_COMP_QUE_MASK    0xFFF0 /* 0b1111 1111 1111 1100 */
#define ME_ADS111x_CONF_COMP_QUE_ONE     0x0000 /* 0b0000 0000 0000 0000 */
#define ME_ADS111x_CONF_COMP_QUE_TWO     0x0001 /* 0b0000 0000 0000 0001 */
#define ME_ADS111x_CONF_COMP_QUE_FOUR    0x0002 /* 0b0000 0000 0000 0010 */
#define ME_ADS111x_CONF_COMP_QUE_DIS     0x0003 /* 0b0000 0000 0000 0011 */

#endif

