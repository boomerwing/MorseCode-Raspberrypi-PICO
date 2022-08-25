/**
 * RP2040 FreeRTOS Template
 *
 * @copyright 2022, Tony Smith (@smittytone)
 * @version   1.2.0
 * @license   MIT
 *
 */
#ifndef _MAIN_H_
#define _MAIN_H_


// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <semphr.h>
// C
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <ctype.h>
#include <time.h>
 /*
 // c++
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cstdint>
 */ 

// Pico SDK
#include "pico/stdlib.h"            // Includes `hardware_gpio.h`
#include "pico/binary_info.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * CONSTANTS
 * 
 */
 
#define         CW1_TIMER_ID            0
#define         CW2_TIMER_ID            0

#define	A0_PIN	26
#define	A1_PIN	27
#define	A2_PIN	28
#define	A3_PIN	29
#define PICO_LED_PIN   25
#define	PINA 18  // 24
#define	PINB 21  // 27
#define	PINC 12  // 16
#define	PIND 14  // 19
#define	PINE 10  // 14
#define	PINF 19  // 25
#define	PING 13  // 17
#define	DOTL 11  // 15
#define	DOTR 16  // 21
#define	SW1  22  // 29
#define	SW2  20  // 26
#define	SW3  17  // 21


// Set a delay time 
const TickType_t ms_delay5  =    5 / portTICK_PERIOD_MS;
const TickType_t ms_delay10 =   10 / portTICK_PERIOD_MS;
const TickType_t ms_delay20 =   20 / portTICK_PERIOD_MS;
const TickType_t ms_delay30 =   30 / portTICK_PERIOD_MS;
const TickType_t ms_delay50 =   50 / portTICK_PERIOD_MS;
const TickType_t ms_delay75 =   75 / portTICK_PERIOD_MS;
const TickType_t ms_delay100 = 100 / portTICK_PERIOD_MS;
const TickType_t ms_delay150 = 150 / portTICK_PERIOD_MS;
const TickType_t ms_delay200 = 200 / portTICK_PERIOD_MS;
const TickType_t ms_delay300 = 300 / portTICK_PERIOD_MS;
const TickType_t ms_delay400 = 400 / portTICK_PERIOD_MS;
const TickType_t ms_delay237 = 237 / portTICK_PERIOD_MS;
const TickType_t ms_delay349 = 349 / portTICK_PERIOD_MS;
const TickType_t ms_delay173 = 173 / portTICK_PERIOD_MS;
const TickType_t ms_delay317 = 317 / portTICK_PERIOD_MS;

/**
 * PROTOTYPES
 */
 
void led_0(void);
void led_1(void);
void led_2(void);
void led_3(void);
void led_4(void);
void led_5(void);
void led_6(void);
void led_7(void);
void led_8(void);
void led_9(void);
void pinf_led_task(void* unused_arg);
void pine_led_task(void* unused_arg);
void pind_led_task(void* unused_arg);
void pinc_led_task(void* unused_arg);
void cw_task(void* unused_arg);
void txtctl_task(void* unused_arg);
void select_phrase_task(void* unused_arg);
void adc_task(void* unused_arg);
void shuffle_task(void* unused_arg);
void random_task(void* unused_arg);
void print_msg(const char* msg);
uint32_t get_number(void);
void send_CW(const uint8_t);
void log_device_info(void);
void configure_gpio(void);
// char *get_string(const char *prompt);
void *get_string(char *text_out_buffer, const char *prompt);
void cw1_timer_fired_callback(TimerHandle_t timer);
void cw2_timer_fired_callback(TimerHandle_t timer);
// void show_seven_seg(uint8_t value);
void sw1_debounce(void* unused_arg);
void sw2_debounce(void* unused_arg);
void sw3_debounce(void* unused_arg);
void sw4_debounce(void* unused_arg);

#ifdef __cplusplus
}           // extern "C"
#endif


#endif      // _MAIN_H_
