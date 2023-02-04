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
 
#define  PHRASE_TIMER_ID  0
#define  DIT_TIMER_ID  0

#define	A0_PIN	26
#define	A1_PIN	27
#define	A2_PIN	28
#define	A3_PIN	29
#define PICO_LED_PIN   25
#define	DOTL 11  // 15
#define	DOTR 16  // 21
#define	SW1  14  // 19
#define	SW2  15  // 20
#define	SW3  17  // 22


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
 
void pinf_led_task(void* unused_arg);
void dots_led_task(void* unused_arg);
void txtctl_task(void* unused_arg);
void on_off_task(void* unused_arg);
void get_string(char* name, uint8_t size);
void log_device_info(void);
void printstr(const char* msg);
void log_debug(const char* msg);

#ifdef __cplusplus
}           // extern "C"
#endif


#endif      // _MAIN_H_
