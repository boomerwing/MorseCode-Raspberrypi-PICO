/**
 * RP2040 FreeRTOS Template
 * 
 * @copyright 2022, Calvin McCarthy
 * @version   1.0.0
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

 void show_seven_seg_num(uint8_t num) {

    switch(num) {   // Seven Segment Display units
        case 0:
            led_0();
            break;
        case 1:
            led_1();
            break;
        case 2:
            led_2();
            break;
        case 3:
            led_3();
            break;
        case 4:
            led_4();
            break;
        case 5:
            led_5();
            break;
        case 6:
            led_6();
            break;
        case 7:
            led_7();
            break;
        case 8:
            led_8();
            break;
        case 9:
            led_9();
            break;
        }
}

/* 
 * @brief Enter number, display on Seven Segment LED 
 * 
 */

 void show_seven_seg_hex(uint8_t num) {

    switch(num) {   // Seven Segment Display units
        case 0:
//            led_0();
                  gpio_put_masked(0X2C5800,0)
            break;
        case 1:
//            led_1();
                  gpio_put_masked(0X201000,0)
            break;
        case 2:
            led_2();
            break;
        case 3:
            led_3();
            break;
        case 4:
            led_4();
            break;
        case 5:
            led_5();
            break;
        case 6:
            led_6();
            break;
        case 7:
            led_7();
            break;
        case 8:
            led_8();
            break;
        case 9:
            led_9();
            break;
        case 0xA:
            led_A();
            break;
        case 0xB:
            led_B();
            break;
        case 0xC:
            led_C();
            break;
        case 0xD:
            led_D();
            break;
        case 0xE:
            led_E();
            break;
        case 0xF:
            led_F();
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

    // Configure the Pin LED
    gpio_init(PINA);
    gpio_set_dir(PINA, GPIO_OUT);
    gpio_disable_pulls(PINA);  // remove pullup and pulldowns
    
    // Configure the Pin LED
    gpio_init(PINB);
    gpio_set_dir(PINB, GPIO_OUT);
    gpio_disable_pulls(PINB);  // remove pullup and pulldowns
    
    // Configure the Pin LED
    gpio_init(PINC);
    gpio_set_dir(PINC, GPIO_OUT);
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
    
}

