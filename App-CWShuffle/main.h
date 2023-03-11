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
 
#define         PAUSE_TIMER_ID    		0
#define         DOT_TIMER_ID            0
#define         ADC_TIMER_ID            0

#define			D0_PIN		           0
#define			D1_PIN			       1
#define			D2_PIN		           2
#define			D3_PIN		           3
#define			D4_PIN			       6
#define			D5_PIN			       7
#define			D6_PIN			       8
#define			D9_PIN		           9
#define			D10_PIN			      10
#define			D11_PIN			      11
#define			D12_PIN		          12
#define			D13_PIN		          13
#define			D19_PIN		          19
#define			D20_PIN		          20
#define			D24_PIN		          24
#define			D25_PIN			      25
#define			D26_PIN	          	  26
#define			D27_PIN	              27
#define			D28_PIN	              28
#define			D29_PIN	              29
#define			A0_PIN	          	  26
#define			A1_PIN	              27
#define			A2_PIN	              28
#define			A3_PIN	              29
#define         Feather_LED_PIN       13
#define         PICO_LED_PIN          25
#define         LED			          24
#define			SW4			           9
#define			SW3			          10
#define			SW2			          11
#define			SW1			          12
#define			TX_PIN		           0
#define			RX_PIN			       1
#define			SDA_PIN		           2
#define			SCL_PIN		           3
#define			SCK_PIN		          18
#define			MO_PIN		          19
#define			MI_PIN		          20

// Set a delay time of exactly 500ms
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
 
void cw_task(void* unused_arg);
void adc_task(void* unused_arg);
void shuffle_task(void* unused_arg);
void print_msg(const char* msg);
void send_CW(const char);
void cw1_timer_fired_callback(TimerHandle_t timer);
void cw2_timer_fired_callback(TimerHandle_t timer);
void configure_gpio(void);
#ifdef __cplusplus
}           // extern "C"
#endif


#endif      // _MAIN_H_
