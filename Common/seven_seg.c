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
#include "../Common/seven_seg.h"
#include "../Common/pcf8575i2c.h"



/* 
 * @brief Enter dec or hex number, display on Seven Segment LED 
 * 
 */

 void show_seven_seg_i2c(uint8_t num) {
    uint8_t  buf1[2]={0b11111111,0b11111111};// data buffer, must be two bytes

    switch(num) {   // Seven Segment Display units
        case 0:
            buf1[1] = 0B01000000; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 1:
            buf1[1] = 0B01111001; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 2:
            buf1[1] = 0B00100100; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 3:
            buf1[1] = 0B10110000; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 4:
            buf1[1] = 0B10011001; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 5:
            buf1[1] = 0B10010010; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 6:
            buf1[1] = 0B10000010; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 7:
            buf1[1] = 0B11111000; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 8:
            buf1[1] = 0B10000000; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 9:
            buf1[1] = 0B10010000; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 0xA:
            buf1[1] = 0B10001000; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 0xB:
            buf1[1] = 0B10000011; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 0xC:
           buf1[1] = 0B11000110; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 0xD:
            buf1[1] = 0B10100001; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 0xE:
            buf1[1] = 0B10000110; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 0xF:
            buf1[1] = 0B10001110; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        case 0x14:
            buf1[1] = 0B11111111; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
            break;
        }
}


/**
 * @brief Initialize GPIO Pin for input and output.
 *
 */
void config_seven_seg(void) {
    uint8_t  buf1[2]={0b11111111,0b11111111};// data buffer, must be two bytes

            buf1[1] = 0B11111111; //  
            i2c_write_blocking(i2c0, I2C_ADDR, buf1, 2, true);
    
}

