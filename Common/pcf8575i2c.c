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
#include "pcf8575i2c.h"

/**
 * @brief pcf8575 init
 */
void pcf8575_init(){
    uint8_t buf[2]={0b11111111,0b11111111};// data buffer, must be two bytes
    i2c_init(i2c_default, 100 * 1000);  // 100000 b/s
    gpio_set_function(GPIO4, GPIO_FUNC_I2C);
    gpio_set_function(GPIO5, GPIO_FUNC_I2C);
    gpio_pull_up(GPIO4);
    gpio_pull_up(GPIO5);
    buf[0]= 0b11111111;
    buf[1]= 0b11111111;
}
    
/**
 * @brief Set new port Byte value modified by OR mask
 */
uint8_t setBit_High(uint8_t inp, uint8_t portnum){
    const uint8_t high_mask[8]={0B00000001,0B00000010,0B00000100,0B00001000,0B00010000,0B00100000,0B01000000,0B10000000};
    return (inp |= high_mask[portnum]);  // OR mask with data to set portnum bit HIGH
}
    
/**
 * @brief Set new port Byte value modified by AND mask
 *
 */
uint8_t setBit_Low(uint8_t inp, uint8_t portnum){
    const uint8_t low_mask[8]={0B11111110,0B11111101,0B11111011,0B11110111,0B11101111,0B11011111,0B10111111,0B01111111};
    return (inp &= low_mask[portnum]);  // AND mask with data to set portnum bit LOW

}
/**
 * @brief Isolate bit value by OR mask to read one port value
 */
uint8_t readBit(uint8_t inp, uint8_t portnum){
    uint8_t value;
    const uint8_t isolate_mask[8]={0B00000001,0B00000010,0B00000100,0B00001000,0B00010000,0B00100000,0B01000000,0B10000000};
    value = (inp &= isolate_mask[portnum]); // AND mask to isolate portnum bit
    if(!value)  {value = 0;}
    else {value = 1;}
    return value; // 1 or 0 output
}
/**
 * @brief Isolate bit value by OR mask to read one port value
 */
uint8_t readBits(uint8_t inp, uint8_t mask){
    uint8_t value;
    value = (inp &= mask); // AND mask to isolate portnum bits
    return value; // 
}

    
