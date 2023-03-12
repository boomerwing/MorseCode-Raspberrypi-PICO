/**
 * RP2040 FreeRTOS seven segment display
 * 
 * @copyright Calvin McCarthy
 * @version   1.2.0
 * @licence   MIT
 *
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "../Common/seven_seg.h"

/**
 * @brief Mask for lighting Seven Seg led.
 *
 */
//  led_0:  0000|0000|0001|1100|0000|0000|1000|1010  0x001C008A
//  led_1:  0000|0000|000x|1100|0000|0000|x000|xxx0  0x0000000A
//  led_2:  0000|0000|000x|1100|0000|0000|1000|11x0  0x000C018C
//  led_3:  0000|0000|000x|x100|0000|0000|1000|1110  0x0004008E
//  led_4:  0000|0000|000x|xx00|0000|0000|x000|xxx0  0x0010000E
//  led_5:  0000|0000|0001|x100|0000|0000|1000|0110  0x00140086
//  led_6:  0000|0000|0001|1100|0000|0000|1000|0110  0x001C0086
//  led_7:  0000|0000|000x|xx00|0000|0000|1000|1x10  0x0000008A
//  led_8:  0000|0000|0001|1100|0000|0000|1000|1110  0x001C008E
//  led_9:  0000|0000|0001|xx00|0000|0000|x000|xxx0  0x0010008E
//  led_A:  0000|0000|0001|1x00|0000|0000|1000|1110  0x0018008E
//  led_B:  0000|0000|0001|1100|0000|0000|x000|x110  0x001C0006
//  led_C:  0000|0000|0001|1100|0000|0000|1000|xxx0  0x001C0080
//  led_D:  0000|0000|000x|1100|0000|0000|x000|1110  0x000C000E
//  led_E:  0000|0000|0001|1100|0000|0000|1000|0100  0x001C0084
//  led_F:  0000|0000|000x|xx00|0000|0000|x000|xxx0  0x001C008E

/**
 * @brief Initialize GPIO Pin for output for Seven Seg LED
 *  Each LED is initialized individually for Out, Pulls Disabled.
 *
 */
void config_seven_seg(void) {

    // Configure the D5/A1 Pin LED
    gpio_init(D5_PIN);
    gpio_set_dir(D5_PIN, GPIO_OUT);
    gpio_disable_pulls(D5_PIN);  // remove pullup and pulldowns
    
     // Configure the SCL/B13 Pin LED
    gpio_init(SCL_PIN);
    gpio_set_dir(SCL_PIN, GPIO_OUT);
    gpio_disable_pulls(SCL_PIN);  // remove pullup and pulldowns
    
    // Configure the RX/C10 Pin LED
    gpio_init(RX_PIN);
    gpio_set_dir(RX_PIN, GPIO_OUT);
    gpio_disable_pulls(RX_PIN);  // remove pullup and pulldowns
    
     // Configure the SCK/D8 Pin LED
    gpio_init(SCK_PIN);
    gpio_set_dir(SCK_PIN, GPIO_OUT);
    gpio_disable_pulls(SCK_PIN);  // remove pullup and pulldowns
    
    // Configure the MO/E7 Pin LED
    gpio_init(MO_PIN);
    gpio_set_dir(MO_PIN, GPIO_OUT);
    gpio_disable_pulls(MO_PIN);  // remove pullup and pulldowns
    
    // Configure the MI/F2 Pin LED
    gpio_init(MI_PIN);
    gpio_set_dir(MI_PIN, GPIO_OUT);
    gpio_disable_pulls(MI_PIN);  // remove pullup and pulldowns
    
     // Configure the SDA/G11 Pin LED
    gpio_init(SDA_PIN);
    gpio_set_dir(SDA_PIN, GPIO_OUT);
    gpio_disable_pulls(SDA_PIN);  // remove pullup and pulldowns
    
    // Configure the TX/RDOT Pin LED
    gpio_init(TX_PIN);
    gpio_set_dir(TX_PIN, GPIO_OUT);
    gpio_disable_pulls(TX_PIN);  // remove pullup and pulldowns
    
    // Configure the D4/LDOT Pin LED
    gpio_init(D4_PIN);
    gpio_set_dir(D4_PIN, GPIO_OUT);
    gpio_disable_pulls(D4_PIN);  // remove pullup and pulldowns
    
    // Configure the D6 Pin LED
    gpio_init(D6_PIN);
    gpio_set_dir(D6_PIN, GPIO_OUT);
    gpio_disable_pulls(D6_PIN);  // remove pullup and pulldowns
    
    
}


/* 
 * @brief Display Binary Number on Seven Seg LED
 * Enter a Binary number, either decimal or HEX 
 * 
 */

 void show_seven_seg(uint8_t num) {

    switch(num) {   // Seven Segment Display units
        case 0:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x001C008A);
            break;
        case 1:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x0000000A);
            break;
        case 2:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x000C008C);
            break;
        case 3:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x0004008E);
            break;
        case 4:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x0010000E);
            break;
        case 5:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x00140086);
            break;
        case 6:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x001C0086);
            break;
        case 7:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x0000008A);
            break;
        case 8:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x001C008E);
            break;
        case 9:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x0010008E);
            break;
        case 0xA:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x0018008E);
            break;
        case 0xB:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x001C0006);
            break;
        case 0xC:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x001C0080);
            break;
        case 0xD:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x000C000E);
            break;
        case 0xE:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x001C0084);
            break;
        case 0xF:
            gpio_set_mask(0x001C008E);
            gpio_clr_mask(0x00180085);
            break;
        }
}

/* 
 * @brief Enter a Binary number, display on Seven Segment LED 
 *  This function looks for a number greater than 0X0F.
 *  Output is shown on separate LEDs as binary 0 to 11
 *  meaning 0x00, 0x10, 0x20, 0x30
 */
void show_seven_seg_hex(uint8_t num) {
    int decade = 0;
    int units = 0;
    decade = num / 16;
    units = num % 16;
    
    switch(decade) {   // Seven Segment Display decade
        case 0x0:
            gpio_put(D4_PIN, 1);  //  DOTR
            gpio_put(TX_PIN, 1);  // DOTL
            break;
        case 0x1:
            gpio_put(D4_PIN, 1);
            gpio_put(TX_PIN, 0);
            break;
        case 0x2:
            gpio_put(D4_PIN, 0);
            gpio_put(TX_PIN, 1);
            break;
        case 0x3:
            gpio_put(D4_PIN, 0);
            gpio_put(TX_PIN, 0);
            break;
        }
    show_seven_seg(units);
}


/* 
 * @brief Enter a Binary number, display on Seven Segment LED 
 *  This function looks for a number greater than 9.
 *  Output is shown on separate LEDs as binary 0 to 11
 *  meaning 0, 10, 20,    30
 */
void show_seven_seg_dec(uint8_t num) {
    int decade = 0;
    int units = 0;
    decade = num / 10;
    units = num % 10;
    
    switch(decade) {   // Seven Segment Display decade
        case 0x0:
            gpio_put(D4_PIN, 1); // DOTR
            gpio_put(TX_PIN, 1); // DOTL
            break;
        case 0x1:
            gpio_put(D4_PIN, 1);
            gpio_put(TX_PIN, 0);
            break;
        case 0x2:
            gpio_put(D4_PIN, 0);
            gpio_put(TX_PIN, 1); 
            break;
        case 0x3:
            gpio_put(D4_PIN, 0);
            gpio_put(TX_PIN, 0);
            break;
        }
    show_seven_seg(units);
}

