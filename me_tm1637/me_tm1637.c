/**
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
    0x7C, /* X111 1100 - b */
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
    0x5C, /* X101 1100 - o */
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

static void me_tm1637_start(me_tm1637_led_t *led);
static void me_tm1637_stop(me_tm1637_led_t *led);
static void me_tm1637_send_byte(me_tm1637_led_t *led, uint8_t byte);
static void me_tm1637_send_buf(me_tm1637_led_t * led, uint8_t *buf);

static const uint8_t seg_pos[][8] = {
                    { 3, 2, 1, 0, TM1637_SEG_MAX, TM1637_SEG_MAX, TM1637_SEG_MAX, TM1637_SEG_MAX },
                    { 3, 4, 5, 0, 1, 2, TM1637_SEG_MAX, TM1637_SEG_MAX }
};

//#define me_tm1637_delay()          ets_delay_us(3)
#define me_tm1637_delay()          ets_delay_us(20)

static void me_tm1637_start(me_tm1637_led_t * led)
{
    gpio_set_level(led->m_pin_dta, 1);
    gpio_set_level(led->m_pin_clk, 1);
    me_tm1637_delay();
    gpio_set_level(led->m_pin_dta, 0);
    me_tm1637_delay();
}

static void me_tm1637_stop(me_tm1637_led_t * led)
{
    gpio_set_level(led->m_pin_clk, 1);
    me_tm1637_delay();
    gpio_set_level(led->m_pin_dta, 1);
    me_tm1637_delay();
}

static void me_tm1637_send_byte(me_tm1637_led_t * led, uint8_t byte)
{
    uint8_t bit = 8;
    //gpio_set_level(led->m_pin_clk, 1);
    while( bit-- )
    {
        gpio_set_level(led->m_pin_clk, 0);
        me_tm1637_delay();
        gpio_set_level(led->m_pin_dta, byte & 0x01);
        me_tm1637_delay();
        gpio_set_level(led->m_pin_clk, 1);
        me_tm1637_delay();
        byte >>= 1;
    }

    // TODO: We can check ACK by reversing output.
    //gpio_set_direction(led->m_pin_dta, GPIO_MODE_INPUT);
    gpio_set_level(led->m_pin_clk, 0); // ACK begins (TM pulls DIO low)
    gpio_set_level(led->m_pin_dta, 0); // to not interfere
    me_tm1637_delay();
    gpio_set_level(led->m_pin_clk, 1); // ACK ends (TM releases DIO)
    me_tm1637_delay();
    gpio_set_level(led->m_pin_clk, 0);
    me_tm1637_delay();
    //gpio_set_direction(led->m_pin_dta, GPIO_MODE_OUTPUT);
}

static void me_tm1637_send_buf(me_tm1637_led_t * led, uint8_t *buf)
{
    const uint8_t *seg = led->seq;

    me_tm1637_start(led);
    me_tm1637_send_byte(led, TM1637_ADDR_AUTO);
    me_tm1637_stop(led);

    me_tm1637_start(led);
    me_tm1637_send_byte(led, 0xc0);
    while( *seg++ != TM1637_SEG_MAX )
        me_tm1637_send_byte(led, *buf++);
    me_tm1637_stop(led);
}

// PUBLIC

void me_tm1637_reset(me_tm1637_led_t *led)
{

    gpio_set_direction(led->m_pin_clk, GPIO_MODE_OUTPUT);
    gpio_set_direction(led->m_pin_dta, GPIO_MODE_OUTPUT);

    gpio_set_level(led->m_pin_clk, 0);
    gpio_set_level(led->m_pin_dta, 0);
    me_tm1637_delay();
    gpio_set_level(led->m_pin_clk, 1);
    me_tm1637_delay();
    gpio_set_level(led->m_pin_dta, 1);
    me_tm1637_delay();
    me_tm1637_set_brightness(led, 0x88);
}

me_tm1637_led_t *me_tm1637_init(gpio_num_t pin_clk, gpio_num_t pin_data, me_tm1637_led_model_t model)
{
    me_tm1637_led_t * led = (me_tm1637_led_t *) malloc(sizeof(me_tm1637_led_t));

    led->m_pin_clk      = pin_clk;
    led->m_pin_dta      = pin_data;
    led->seq            = seg_pos[model];

    // Set CLK to low during DIO initialization to avoid sending a start signal by mistake
    me_tm1637_reset(led);

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
    uint8_t data;

    data = ch < sizeof(me_tm1637_symbols) ? me_tm1637_symbols[ch] : 0;
    if( dot )
        data |= 0x80;

    me_tm1637_start(led);
    me_tm1637_send_byte(led, TM1637_ADDR_FIXED);
    me_tm1637_stop(led);
    me_tm1637_start(led);
    me_tm1637_send_byte(led, segment_idx | 0xc0);
    me_tm1637_send_byte(led, data);
    me_tm1637_stop(led);
}


void me_tm1637_set_number_dot(me_tm1637_led_t * led, int32_t number, bool lead_zero, int8_t dot_pos)
{
    const uint8_t     *seq = led->seq;
    uint8_t            seg = *seq;
    uint8_t            is_neg = 0, ch;
    uint8_t            buf[6] = {0,0,0,0,0,0};

    if( number < 0 )
    {
        is_neg = 1;
        number *= (-1);
    }

    while( 1 )
    {
        ch = number % 10;
        ch = ch < sizeof(me_tm1637_symbols) ? me_tm1637_symbols[ch] : 0;
        if( dot_pos-- == 0 )
            ch |= 0x80;
        buf[seg] = ch;

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
        buf[seg] = TM1637_SYM_MINUS;
    }

    me_tm1637_send_buf(led, buf);
}

void me_tm1637_set_text(me_tm1637_led_t * led, uint8_t *text, int8_t chars)
{
    const uint8_t     *seq = led->seq;
    uint8_t            seg = *seq;
    uint8_t            ch, dot = 0;
    uint8_t            buf[6] = {0,0,0,0,0,0};
    

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

        ch = ch < sizeof(me_tm1637_symbols) ? me_tm1637_symbols[ch] : 0;
        if( dot )
        {
            ch |= 0x80;
            dot = 0;
        }
        buf[seg] = ch;

        seg = *(++seq);
        if( seg == TM1637_SEG_MAX )
            break;
    }
    me_tm1637_send_buf(led, buf);
}
