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
#include "../Common/seven_seg.h"


/* 
 * @brief Enter Binary number 
 * 
 */


/* 
 * @brief Enter number, display on Seven Segment LED 
 * 
 */

 void show_seven_seg(uint8_t num) {

    switch(num) {   // Seven Segment Display units
        case 0:
            gpio_set_mask(0X2C7400);
            gpio_clr_mask(0X2C5400);
            break;
        case 1:
            gpio_set_mask(0X2C7400); 
            gpio_clr_mask (0X201000);
            break;
        case 2:
            gpio_set_mask(0X2C7400); 
            gpio_clr_mask(0X246400);
            break;
        case 3:
            gpio_set_mask(0X2C7400); 
            gpio_clr_mask(0X247800);
            break;
        case 4:
            gpio_set_mask(0X2C7400); 
            gpio_clr_mask(0X283000);
            break;
        case 5:
            gpio_set_mask(0X2C7400); 
            gpio_clr_mask(0XC7000);
            break;
        case 6:
            gpio_set_mask(0X2C7400); 
            gpio_clr_mask(0XC7400);
            break;
        case 7:
            gpio_set_mask(0X2C7400); 
            gpio_clr_mask(0X241000);
            break;
        case 8:
            gpio_set_mask(0X2C7400); 
            gpio_clr_mask(0X2C7400);
            break;
        case 9:
            gpio_set_mask(0X2C7400); 
            gpio_clr_mask(0X2C7000);
            break;
        case 0xA:
            gpio_set_mask(0X2C7400); 
            gpio_clr_mask(0X4C3400);
            break;
        case 0xB:
            gpio_set_mask(0X2C7400); 
            gpio_clr_mask(0X87400);
            break;
        case 0xC:
            gpio_set_mask(0X2C7400); 
            gpio_clr_mask(0XC4400);
            break;
        case 0xD:
            gpio_set_mask(0X2C7400); 
            gpio_clr_mask(0X40B400);
            break;
        case 0xE:
            gpio_set_mask(0X2C7400); 
            gpio_clr_mask(0XC6400);
            break;
        case 0xF:
            gpio_set_mask(0X2C7400); 
            gpio_clr_mask(0XC2400);
            break;
        }
}

/**
 * @brief Light a Seven Segment LED with the numaber 0
 *
 */
void led_0(void) {
            gpio_put(PINA, 0);  // A1
            gpio_put(PINB, 0); // B13
            gpio_put(PINC, 0);  // C10
            gpio_put(PIND, 0); // D8
            gpio_put(PINE, 0);  // E7
            gpio_put(PINF, 0);  // F2
            gpio_put(PING, 1); // G11
}

/**
 * @brief Light a Seven Segment LED with the numaber 1
 *
 */
void led_1(void) {
            gpio_put(PINA, 1);  // A1
            gpio_put(PINB, 1); // B13
            gpio_put(PINC, 1);  // C10
            gpio_put(PIND, 1); // D8
            gpio_put(PINE, 0);  // E7
            gpio_put(PINF, 0);  // F2
            gpio_put(PING, 1); // G11
}

/**
 * @brief Light a Seven Segment LED with the numaber 2
 *
 */
void led_2(void) {
            gpio_put(PINA, 0);  // A1
            gpio_put(PINB, 0); // B13
            gpio_put(PINC, 1);  // C10
            gpio_put(PIND, 0); // D8
            gpio_put(PINE, 0);  // E7
            gpio_put(PINF, 1);  // F2
            gpio_put(PING, 0); // G11
}

/**
 * @brief Light a Seven Segment LED with the numaber 3
 *
 */
void led_3(void) {
            gpio_put(PINA, 0);  // A1
            gpio_put(PINB, 0); // B13
            gpio_put(PINC, 0);  // C10
            gpio_put(PIND, 0); // D8
            gpio_put(PINE, 1);  // E7
            gpio_put(PINF, 1);  // F2
            gpio_put(PING, 0); // G11
}

/**
 * @brief Light a Seven Segment LED with the numaber 4
 *
 */
void led_4(void) {
            gpio_put(PINA, 1);  // A1
            gpio_put(PINB, 0); // B13
            gpio_put(PINC, 0);  // C10
            gpio_put(PIND, 1); // D8
            gpio_put(PINE, 1);  // E7
            gpio_put(PINF, 0);  // F2
            gpio_put(PING, 0); // G11
}

/**
 * @brief Light a Seven Segment LED with the numaber 5
 *
 */
void led_5(void) {
            gpio_put(PINA, 0);  // A1
            gpio_put(PINB, 1); // B13
            gpio_put(PINC, 0);  // C10
            gpio_put(PIND, 0); // D8
            gpio_put(PINE, 1);  // E7
            gpio_put(PINF, 0);  // F2
            gpio_put(PING, 0); // G11
}

/**
 * @brief Light a Seven Segment LED with the numaber 6
 *
 */
void led_6(void) {
            gpio_put(PINA, 0);  // A1
            gpio_put(PINB, 1); // B13
            gpio_put(PINC, 0);  // C10
            gpio_put(PIND, 0); // D8
            gpio_put(PINE, 0);  // E7
            gpio_put(PINF, 0);  // F2
            gpio_put(PING, 0); // G11
}

/**
 * @brief Light a Seven Segment LED with the numaber 7
 *
 */
void led_7(void) {
            gpio_put(PINA, 0);  // A1
            gpio_put(PINB, 0); // B13
            gpio_put(PINC, 0);  // C10
            gpio_put(PIND, 1); // D8
            gpio_put(PINE, 1);  // E7
            gpio_put(PINF, 1);  // F2
            gpio_put(PING, 1); // G11
}

/**
 * @brief Light a Seven Segment LED with the numaber 8
 *
 */
void led_8(void) {
            gpio_put(PINA, 0);  // A1
            gpio_put(PINB, 0); // B13
            gpio_put(PINC, 0);  // C10
            gpio_put(PIND, 0); // D8
            gpio_put(PINE, 0);  // E7
            gpio_put(PINF, 0);  // F2
            gpio_put(PING, 0); // G11
}

/**
 * @brief Light a Seven Segment LED with the numaber 9
 *
 */
void led_9(void) {
            gpio_put(PINA, 0);  // A1
            gpio_put(PINB, 0); // B13
            gpio_put(PINC, 0);  // C10
            gpio_put(PIND, 1); // D8
            gpio_put(PINE, 1);  // E7
            gpio_put(PINF, 0);  // F2
            gpio_put(PING, 0); // G11
}

/**
 * @brief Light a Seven Segment LED with the numaber A
 *
 */
void led_A(void) {
            gpio_put(PINA, 0);  // A1
            gpio_put(PINB, 0); // B13
            gpio_put(PINC, 0);  // C10
            gpio_put(PIND, 1); // D8
            gpio_put(PINE, 0);  // E7
            gpio_put(PINF, 0);  // F2
            gpio_put(PING, 0); // G11
}

/**
 * @brief Light a Seven Segment LED with the numaber B
 *
 */
void led_B(void) {
            gpio_put(PINA, 1);  // A1
            gpio_put(PINB, 1); // B13
            gpio_put(PINC, 0);  // C10
            gpio_put(PIND, 0); // D8
            gpio_put(PINE, 0);  // E7
            gpio_put(PINF, 0);  // F2
            gpio_put(PING, 0); // G11
}

/**
 * @brief Light a Seven Segment LED with the numaber C
 *
 */
void led_C(void) {
            gpio_put(PINA, 0);  // A1
            gpio_put(PINB, 1); // B13
            gpio_put(PINC, 1);  // C10
            gpio_put(PIND, 0); // D8
            gpio_put(PINE, 0);  // E7
            gpio_put(PINF, 0);  // F2
            gpio_put(PING, 1); // G11
}

/**
 * @brief Light a Seven Segment LED with the numaber D
 *
 */
void led_D(void) {
            gpio_put(PINA, 1);  // A1
            gpio_put(PINB, 0); // B13
            gpio_put(PINC, 0);  // C10
            gpio_put(PIND, 0); // D8
            gpio_put(PINE, 0);  // E7
            gpio_put(PINF, 1);  // F2
            gpio_put(PING, 0); // G11
}

/**
 * @brief Light a Seven Segment LED with the numaber E
 *
 */
void led_E(void) {
            gpio_put(PINA, 0);  // A1
            gpio_put(PINB, 1); // B13
            gpio_put(PINC, 1);  // C10
            gpio_put(PIND, 0); // D8
            gpio_put(PINE, 0);  // E7
            gpio_put(PINF, 0);  // F2
            gpio_put(PING, 0); // G11
}

/**
 * @brief Light a Seven Segment LED with the numaber F
 *
 */
void led_F(void) {
            gpio_put(PINA, 0);  // A1
            gpio_put(PINB, 1); // B13
            gpio_put(PINC, 1);  // C10
            gpio_put(PIND, 1); // D8
            gpio_put(PINE, 0);  // E7
            gpio_put(PINF, 0);  // F2
            gpio_put(PING, 0); // G11
}

/**
 * @brief Initialize GPIO Pin for input and output.
 *
 */
void config_seven_seg(void) {

    gpio_init_mask(0X2C7400);
    gpio_set_dir_out_masked(0X2C7400);  // all seven segment outputs


    // Configure the Pin LED
//    gpio_init(PINA);
//    gpio_set_dir(PINA, GPIO_OUT);
    gpio_disable_pulls(PINA);  // remove pullup and pulldowns
    
    // Configure the Pin LED
//    gpio_init(PINB);
//    gpio_set_dir(PINB, GPIO_OUT);
    gpio_disable_pulls(PINB);  // remove pullup and pulldowns
    
    // Configure the Pin LED
//    gpio_init(PINC);
//    gpio_set_dir(PINC, GPIO_OUT);
    gpio_disable_pulls(PINC);  // remove pullup and pulldowns
    
    // Configure the Pin LED
    gpio_init(PIND);
    gpio_set_dir(PIND, GPIO_OUT);
    gpio_disable_pulls(PIND);  // remove pullup and pulldowns
    
    // Configure the Pin LED
    gpio_init(PINE);
    gpio_set_dir(PINE, GPIO_OUT);
    gpio_disable_pulls(PINE);  // remove pullup and pulldowns
    
    // Configure the Pin LED
    gpio_init(PINF);
    gpio_set_dir(PINF, GPIO_OUT);
    gpio_disable_pulls(PINF);  // remove pullup and pulldowns
    
    // Configure the Pin LED
    gpio_init(PING);
    gpio_set_dir(PING, GPIO_OUT);
    gpio_disable_pulls(PING);  // remove pullup and pulldowns
    
    // Configure the Pin LED
    gpio_init(DOTL);
    gpio_set_dir(DOTL, GPIO_OUT);
    gpio_disable_pulls(DOTL);  // remove pullup and pulldowns

    // Configure the Pin LED
    gpio_init(DOTR);
    gpio_set_dir(DOTR, GPIO_OUT);
    gpio_disable_pulls(DOTR);  // remove pullup and pulldowns

    gpio_put_masked(0X2C7400,1);
    
}

