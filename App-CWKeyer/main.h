/**
 * RP2040 FreeRTOS App_Keyer
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
 
#define PICO_LED_PIN   25
#define	DOTL 11  // 15
#define	DOTR 16  // 21
#define	SW1  22  // 29
#define	SW2  20  // 26
#define	SW3  17  // 21
#define PHRASE_TIMER_ID 0
#define DIT_TIMER_ID 0


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
 
void sw1_debounce(void* unused_arg);
void sw2_debounce(void* unused_arg);
void led1_task(void* unused_arg);
void led2_task(void* unused_arg);
void print_msg(const char* msg);
void log_device_info(void);
void configure_gpio(void);
void cw_task(void* unused_arg);
void send_CW(char ascii_in);
void windowT(void);
void windowB(void);
void log_debug(const char* msg);

#ifdef __cplusplus
}           // extern "C"
#endif


#endif      // _MAIN_H_
