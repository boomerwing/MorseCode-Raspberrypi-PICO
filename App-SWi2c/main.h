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
 
#define   PHRASE_TIMER_ID  0
#define   DOT_TIMER_ID     0
#define   T1_TIMER_ID      0
#define   PAUSE_PERIOD  2800
#define   DOT_PERIOD     500

#define	A0_PIN	26
#define	A1_PIN	27
#define	A2_PIN	28
#define	A3_PIN	29
#define PICO_LED_PIN   25
#define	SW11  11  // 19
#define	SW12  17  // 20
#define	SW0   17  // 22
#define	SW1   18  // 24
#define	SW2   19  // 25
#define	DOTL  11  // 15
#define	DOTR  16  // 21
#define	DOTA  20  // 26
#define	DOTF  21  // 27
#define	DOTD  12  // 16
#define	DOTC  13  // 17
#define	ON 0      // 
#define	OFF 1     // 


// Set a delay time 
const TickType_t ms_delay5  =    5 / portTICK_PERIOD_MS;
const TickType_t ms_delay10 =   10 / portTICK_PERIOD_MS;
const TickType_t ms_delay20 =   20 / portTICK_PERIOD_MS;
const TickType_t ms_delay30 =   30 / portTICK_PERIOD_MS;
const TickType_t ms_delay50 =   50 / portTICK_PERIOD_MS;
const TickType_t ms_delay75 =   75 / portTICK_PERIOD_MS;
const TickType_t ms_delay95 =   75 / portTICK_PERIOD_MS;
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
 
void print_msg(const char* msg);
void log_device_info(void);
void configure_gpio(void);
void swP1_debounce(void* unused_arg);
void led1_task(void* unused_arg);
void led2_task(void* unused_arg);
void gate1_task(void* unused_arg);
void Latch_task(void* unused_arg);
void dot_timer_fired_callback(TimerHandle_t timer);
void phrase_timer_fired_callback(TimerHandle_t timer);
void ti_timer_fired_callback(TimerHandle_t timer);
uint32_t get_delay(void);

#ifdef __cplusplus
}           // extern "C"
#endif


#endif      // _MAIN_H_
