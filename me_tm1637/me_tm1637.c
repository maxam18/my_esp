/**
 * Based on Petro's lib https://github.com/petrows/esp-32-me_tm1637
 */


#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <esp32/rom/ets_sys.h>

#include <me_tm1637.h>

#define TM1637_ADDR_AUTO  0x40
#define TM1637_ADDR_FIXED 0x44

#define TM1637_SYM_MINUS    0x40 /* 0b01000000 - '-' */

static const int8_t me_tm1637_symbols[] = {
          /*  GFE DCBA */
    0x3f, /* X011 1111 - 0 */
    0x06, /* X000 0110 - 1 */
    0x5b, /* X101 1011 - 2 */
    0x4f, /* X100 1111 - 3 */
    0x66, /* X110 0110 - 4 */
    0x6d, /* X110 1101 - 5 */
    0x7d, /* X111 1101 - 6 */
    0x07, /* X000 0111 - 7 */
    0x7f, /* X111 1111 - 8 */
    0x6f, /* X110 1111 - 9 */
    0x77, /* X111 0111 - A */
    0x7c, /* X111 1100 - b */
    0x39, /* X011 1001 - C */
    0x5e, /* X101 1110 - d */
    0x79, /* X111 1001 - E */
    0x71, /* X111 0001 - F */
    0x01, /* X000 0001 - cannot show */ 
    0x77, /* X111 0111 - A */
    0x7c, /* X111 1100 - b */
    0x39, /* X011 1001 - C */
    0x5e, /* X101 1110 - d */
    0x79, /* X111 1001 - E */
    0x71, /* X111 0001 - F */
    0x3D, /* X011 1101 - G */
    0x76, /* X111 0110 - H */
    0x30, /* X011 0000 - I */
    0x0E, /* X000 1110 - J */
          /*  GFE DCBA */
    0x01, /* X000 0001 - cannot show */
    0x38, /* X011 1000 - L */
    0x01, /* X000 0001 - cannot show */
    0x54, /* X101 0100 - N */
    0x3F, /* X011 1111 - O */
    0x73, /* X111 0011 - P */
    0x67, /* X110 0111 - Q */
    0x50, /* X101 0000 - R */
    0x6D, /* X110 1101 - S */
    0x78, /* X111 1000 - T */
    0x3E, /* X011 1110 - U */
    0x01, /* X000 0001 - cannot show */
    0x01, /* X000 0001 - cannot show */
    0x01, /* X000 0001 - cannot show */
    0x6E, /* X110 1110 - Y */
    0x01, /* X000 0001 - cannot show */
};

static void me_tm1637_start(me_tm1637_led_t * led);
static void me_tm1637_stop(me_tm1637_led_t * led);
static void me_tm1637_send_byte(me_tm1637_led_t * led, uint8_t byte);

static const uint8_t seg_pos[][8] = {
                    { 3, 2, 1, 0, TM1637_SEG_MAX, TM1637_SEG_MAX, TM1637_SEG_MAX, TM1637_SEG_MAX },
                    { 3, 4, 5, 0, 1, 2, TM1637_SEG_MAX, TM1637_SEG_MAX }
};

#define me_tm1637_delay()          ets_delay_us(3)

void me_tm1637_start(me_tm1637_led_t * led)
{
    // Send start signal
    // Both outputs are expected to be HIGH beforehand
    gpio_set_level(led->m_pin_dta, 0);
    me_tm1637_delay();
}

void me_tm1637_stop(me_tm1637_led_t * led)
{
    // Send stop signal
    // CLK is expected to be LOW beforehand
    gpio_set_level(led->m_pin_dta, 0);
    me_tm1637_delay();
    gpio_set_level(led->m_pin_clk, 1);
    me_tm1637_delay();
    gpio_set_level(led->m_pin_dta, 1);
    me_tm1637_delay();
}

void me_tm1637_send_byte(me_tm1637_led_t * led, uint8_t byte)
{
    for (uint8_t i=0; i<8; ++i)
    {
        gpio_set_level(led->m_pin_clk, 0);
        me_tm1637_delay();
        gpio_set_level(led->m_pin_dta, byte & 0x01); // Send current bit
        byte >>= 1;
        me_tm1637_delay();
        gpio_set_level(led->m_pin_clk, 1);
        me_tm1637_delay();
    }

    // The TM1637 signals an ACK by pulling DIO low from the falling edge of
    // CLK after sending the 8th bit, to the next falling edge of CLK.
    // DIO needs to be set as input during this time to avoid having both
    // chips trying to drive DIO at the same time.
    gpio_set_direction(led->m_pin_dta, GPIO_MODE_INPUT);
    gpio_set_level(led->m_pin_clk, 0); // TM1637 starts ACK (pulls DIO low)
    me_tm1637_delay();
    gpio_set_level(led->m_pin_clk, 1);
    me_tm1637_delay();
    gpio_set_level(led->m_pin_clk, 0); // TM1637 ends ACK (releasing DIO)
    me_tm1637_delay();
    gpio_set_direction(led->m_pin_dta, GPIO_MODE_OUTPUT);
}

// PUBLIC

me_tm1637_led_t *me_tm1637_init(gpio_num_t pin_clk, gpio_num_t pin_data, me_tm1637_led_model_t model) {
    me_tm1637_led_t * led = (me_tm1637_led_t *) malloc(sizeof(me_tm1637_led_t));

    led->m_pin_clk      = pin_clk;
    led->m_pin_dta      = pin_data;
    led->seq            = seg_pos[model];

    // Set CLK to low during DIO initialization to avoid sending a start signal by mistake
    gpio_set_direction(pin_clk, GPIO_MODE_OUTPUT);
    gpio_set_level(pin_clk, 0);
    me_tm1637_delay();
    gpio_set_direction(pin_data, GPIO_MODE_OUTPUT);
    gpio_set_level(pin_data, 1);
    me_tm1637_delay();
    gpio_set_level(pin_clk, 1);
    me_tm1637_delay();

    return led;
}

void me_tm1637_set_brightness(me_tm1637_led_t * led, uint8_t level)
{
    me_tm1637_start(led);
    me_tm1637_send_byte(led, level | 0x88);
    me_tm1637_stop(led);
}

void me_tm1637_set_segment(me_tm1637_led_t * led, const uint8_t segment_idx, const uint8_t ch, const bool dot)
{
    uint8_t seg_data = 0x00;

    if (ch < (sizeof(me_tm1637_symbols)/sizeof(me_tm1637_symbols[0]))) {
        seg_data = me_tm1637_symbols[ch];
    }

    if (dot) {
        seg_data |= 0x80; // Set DOT segment flag
    }

    me_tm1637_set_segment_raw(led, segment_idx, seg_data);
}

void me_tm1637_set_segment_raw(me_tm1637_led_t * led, const uint8_t segment_idx, const uint8_t data)
{
    me_tm1637_start(led);
    me_tm1637_send_byte(led, TM1637_ADDR_FIXED);
    me_tm1637_stop(led);
    me_tm1637_start(led);
    me_tm1637_send_byte(led, segment_idx | 0xc0);
    me_tm1637_send_byte(led, data);
    me_tm1637_stop(led);
}

void me_tm1637_set_number_lead_dot(me_tm1637_led_t * led, int32_t number, bool lead_zero, int8_t dot_pos)
{
    const uint8_t     *seq = led->seq;
    uint8_t            seg = *seq;
    uint8_t            is_neg = 0;

    if( number < 0 )
    {
        is_neg = 1;
        number *= (-1);
    }

    while( 1 )
    {
        me_tm1637_set_segment(led, seg, number % 10, (dot_pos-- == 0));

        number /= 10;
        if( number == 0 && !lead_zero )
            break;

        seg = *(++seq);
        if( seg == TM1637_SEG_MAX )
            break;
    }

    if( is_neg )
    {
        if( seg == TM1637_SEG_MAX )
            seg = *(--seq);
        me_tm1637_set_segment_raw(led, seg, TM1637_SYM_MINUS);
    }
}

void me_tm1637_set_text(me_tm1637_led_t * led, uint8_t *text, int8_t chars)
{
    const uint8_t     *seq = led->seq;
    uint8_t            seg = *seq;
    uint8_t            ch, dot = 0;
    
    while( chars-- )
    {
        ch = text[chars];
        if( ch == '.' ) 
        {
            dot = 1;
            continue;
        } else if( ch >= 'a' && ch <= 'z' )
            ch -= ('a' - 'A' - '0');
        else
            ch -= '0';

        me_tm1637_set_segment(led, seg, ch, dot);
        dot = 0;

        seg = *(++seq);
        if( seg == TM1637_SEG_MAX )
            break;
    }
}
