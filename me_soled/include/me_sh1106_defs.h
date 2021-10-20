/* SH1106 OLED definitions
 * File: me_sh1106_defs.h
 */

#ifndef _ME_SH1106_DEFS_H
#define _ME_SH1106_DEFS_H

// Following definitions are bollowed from 
// http://robotcantalk.blogspot.com/2015/03/interfacing-arduino-with-ssd1306-driven.html

#define ME_SH1106_I2C_ADDRESS                0x3C

// Control bytes
#define ME_SH1106_CMD_SINGLE                 0x80
#define ME_SH1106_CMD_STREAM                 0x00
#define ME_SH1106_REG_DATA_STREAM            0x40

// Commands
#define ME_SH1106_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define ME_SH1106_CMD_DISPLAY_OFF            0xAE
#define ME_SH1106_CMD_DISPLAY_ON             0xAF

// Addressing Command Table (pg.30)
#define ME_SH1106_CMD_SET_MEMORY_ADDR_MODE   0x20    // follow with 0x00 = HORZ mode = Behave like a KS108 graphic LCD
#define ME_SH1106_CMD_SET_HORI_ADDR_MODE     0x00    // Horizontal Addressing Mode
#define ME_SH1106_CMD_SET_VERT_ADDR_MODE     0x01    // Vertical Addressing Mode
#define ME_SH1106_CMD_SET_PAGE_ADDR_MODE     0x02    // Page Addressing Mode
#define ME_SH1106_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define ME_SH1106_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define ME_SH1106_CMD_SET_DISPLAY_START_LINE 0x40
#define ME_SH1106_CMD_SET_SEGMENT_REMAP_ON   0xA0    
#define ME_SH1106_CMD_SET_SEGMENT_REMAP_OFF  0xA1 
#define ME_SH1106_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define ME_SH1106_CMD_SET_MUX_RATIO          0xA8    // follow with 0xFF
#define ME_SH1106_CMD_SET_COM_SCAN_MODE      0xC8    
#define ME_SH1106_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define ME_SH1106_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define ME_SH1106_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define ME_SH1106_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define ME_SH1106_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define ME_SH1106_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14
#define ME_SH1106_CMD_SET_CHARGE_PUMP_CTRL   0xAD    
#define ME_SH1106_CMD_SET_CHARGE_PUMP_ON     0x0B    
#define ME_SH1106_CMD_SET_CHARGE_PUMP_OFF    0x0A

#define ME_SH1106_MAX_ROWS                   8

#endif /* MAIN_SSD1366_H_ */