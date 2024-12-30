/**
 * RP2040 FreeRTOS Template
 * 
 * @copyright 2022, Tony Smith (@smittytone)
 * @version   1.2.0
 * @licence   MIT
 *
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "seven_seg.h"
#include "../pcf8575i2c.h"

#define DOTR 6
#define DOTL 5

/* 
 * @brief Enter dec or hex number, display on Seven Segment LED 
 * 
 */

 void show_seven_seg(uint8_t num) {
    uint8_t  buf[]={0b11111111,0b11111111};// data buffer, must be two bytes

    switch(num) {   // Seven Segment Display units
        case 0:
            buf[1] = 0B01000000; //  
            break;
        case 1:
            buf[1] = 0B01111001; //  
            break;
        case 2:
            buf[1] = 0B00100100; //  
            break;
        case 3:
            buf[1] = 0B10110000; //  
            break;
        case 4:
            buf[1] = 0B10011001; //  
            break;
        case 5:
            buf[1] = 0B10010010; //  
            break;
        case 6:
            buf[1] = 0B10000010; //  
            break;
        case 7:
            buf[1] = 0B11111000; //  
            break;
        case 8:
            buf[1] = 0B10000000; //  
            break;
        case 9:
            buf[1] = 0B10010000; //  
            break;
        case 0xA:
            buf[1] = 0B10001000; //  
            break;
        case 0xB:
            buf[1] = 0B10000011; //  
            break;
        case 0xC:
            buf[1] = 0B11000110; //  
            break;
        case 0xD:
            buf[1] = 0B10100001; //  
            break;
        case 0xE:
            buf[1] = 0B10000110; //  
            break;
        case 0xF:
            buf[1] = 0B10001110; //  
            break;
        case 0x14:
            buf[1] = 0B11111111; //  blank 7 seg display
            break;
        }
      i2c_write_blocking(i2c0, I2C_ADDR, buf, 2, true);
}


/* 
 * @brief Enter 0 to 6, display one segment on Seven Segment LED 
 * 
 */

 void segment_ON(uint8_t num) {
    uint8_t pcfbuffer[]={0b11111111,0b11111111};// data buffer, must be two bytes
    uint8_t pattern_array[]={0,1,2,3,4,5,6};    // step through pattern on 7seg

       i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, false);
        pcfbuffer[1] = setBit_Low(pcfbuffer[1], pattern_array[num]);
        i2c_write_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, false);
}

/* 
 * @brief Enter 0 to 6, blank one segment on Seven Segment LED 
 * 
 */

 void segment_OFF(uint8_t num) {
    uint8_t pcfbuffer[]={0b11111111,0b11111111};// data buffer, must be two bytes
    uint8_t pattern_array[]={0,1,2,3,4,5,6};    // step through pattern on 7seg


       i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, false);
        pcfbuffer[1] = setBit_High(pcfbuffer[1], pattern_array[num]);
        i2c_write_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, false);
}

/* 
 * @brief Turn OFF all  Seven Segment LED segments
 * 
 */

 void blank_seven_seg() {
    uint8_t buf[]={0b11111111,0b11111111};// data buffer, must be two bytes
    
        buf[1] = 0B11111111; //  blank 7 seg display
        i2c_write_blocking(i2c0, I2C_ADDR, buf, 2, false);
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

/**
 * @brief Initialize GPIO Pin for input and output.
 *
 */
void config_seven_seg(void) {
    uint8_t  buf[]={0b11111111,0b11111111};// data buffer, must be two bytes

            buf[1] = 0B11111111; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf, 2, true);
    
}
