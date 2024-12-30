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
 * @brief Initialize GPIO Pin for output for Seven Seg LED
 *  Each LED is initialized individually for Out, Pulls Disabled.
 * 
 *                  A L            R CE   GBFD
 *   0b0000|0000|000x|x000|0000|000x|xx00|xxxx
 * 
 */
void config_seven_seg(void) {

    // Configure the A1 Pin LED
    gpio_init(SA_PIN);
    gpio_set_dir(SA_PIN, GPIO_OUT);
    gpio_disable_pulls(SA_PIN);  // remove pullup and pulldowns
    
     // Configure the B13 Pin LED
    gpio_init(SB_PIN);
    gpio_set_dir(SB_PIN, GPIO_OUT);
    gpio_disable_pulls(SB_PIN);  // remove pullup and pulldowns
    
    // Configure the C10 Pin LED
    gpio_init(SC_PIN);
    gpio_set_dir(SC_PIN, GPIO_OUT);
    gpio_disable_pulls(SC_PIN);  // remove pullup and pulldowns
    
     // Configure the D8 Pin LED
    gpio_init(SD_PIN);
    gpio_set_dir(SD_PIN, GPIO_OUT);
    gpio_disable_pulls(SD_PIN);  // remove pullup and pulldowns
    
    // Configure the E7 Pin LED
    gpio_init(SE_PIN);
    gpio_set_dir(SE_PIN, GPIO_OUT);
    gpio_disable_pulls(SE_PIN);  // remove pullup and pulldowns
    
    // Configure the F2 Pin LED
    gpio_init(SF_PIN);
    gpio_set_dir(SF_PIN, GPIO_OUT);
    gpio_disable_pulls(SF_PIN);  // remove pullup and pulldowns
    
     // Configure the SDA/G11 Pin LED
    gpio_init(SG_PIN);
    gpio_set_dir(SG_PIN, GPIO_OUT);
    gpio_disable_pulls(SG_PIN);  // remove pullup and pulldowns
    
    // Configure the DOTR Pin LED
    gpio_init(DOTR);
    gpio_set_dir(DOTR, GPIO_OUT);
    gpio_disable_pulls(DOTR);  // remove pullup and pulldowns
    
    // Configure the D4/LDOT Pin LED
    gpio_init(DOTL);
    gpio_set_dir(DOTL, GPIO_OUT);
    gpio_disable_pulls(DOTL);  // remove pullup and pulldowns
    
    // Configure the GPI09 Pin LED
    gpio_init(GPIO9);
    gpio_set_dir(GPIO9, GPIO_OUT);
    gpio_disable_pulls(GPIO9);  // remove pullup and pulldowns
    
    
}


/* 
 * @brief Display Binary Number on Seven Seg LED
 * Enter a Binary number, either decimal or HEX 
 * 
 */

 void show_seven_seg(uint8_t num) {

    gpio_set_mask(0x001000CF);
    
    switch(num) {   // Seven Segment Display units
        case 0:
            gpio_clr_mask(0x001000C7);
            break;
        case 1:
            gpio_clr_mask(0x00000084);
            break;
        case 2:
            gpio_clr_mask(0x0010004D);
            break;
        case 3:
            gpio_clr_mask(0x0010008D);
            break;
        case 4:
            gpio_clr_mask(0x0000008E);
            break;
        case 5:
            gpio_clr_mask(0x0010008B);
            break;
        case 6:
           gpio_clr_mask(0x001000CD);
            break;
        case 7:
            gpio_clr_mask(0x00100084);
            break;
        case 8:
            gpio_clr_mask(0x001000CF);
            break;
        case 9:
            gpio_clr_mask(0x0010008F);
            break;
        case 0xA:
            gpio_clr_mask(0x001000CE);
            break;
        case 0xB:
            gpio_clr_mask(0x000000CB);
            break;
        case 0xC:
            gpio_clr_mask(0x00100043);
            break;
        case 0xD:
            gpio_clr_mask(0x000000CD);
            break;
        case 0xE:
            gpio_clr_mask(0x0010004B);
            break;
        case 0xF:
           gpio_clr_mask(0x0010004A);
            break;
        }
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
            gpio_put(DOTR, 1); // DOTR
            gpio_put(DOTL, 1); // DOTL
            break;
        case 0x1:
            gpio_put(DOTR, 1);
            gpio_put(DOTL, 0);
            break;
        case 0x2:
            gpio_put(DOTR, 0);
            gpio_put(DOTL, 1); 
            break;
        case 0x3:
            gpio_put(DOTR, 0);
            gpio_put(DOTL, 0);
            break;
        }
    show_seven_seg(units);
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
            gpio_put(DOTR, 1);  //  DOTR
            gpio_put(DOTL, 1);  // DOTL
            break;
        case 0x1:
            gpio_put(DOTR, 1);
            gpio_put(DOTL, 0);
            break;
        case 0x2:
            gpio_put(DOTR, 0);
            gpio_put(DOTL, 1);
            break;
        case 0x3:
            gpio_put(DOTR, 0);
            gpio_put(DOTL, 0);
            break;
        }
    show_seven_seg(units);
}

