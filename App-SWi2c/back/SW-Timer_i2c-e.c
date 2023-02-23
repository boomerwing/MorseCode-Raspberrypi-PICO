/**
 * RP2040 FreeRTOS Timer play
 * 
 * @copyright 2022, Calvin McCarthy
 * @version   1.0.0
 * @licence   MIT
 *
 */
#include <stdio.h>
#include "main.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "../Common/seven_seg.h"
#include "../Common/pcf8575i2c.h"

/*
 * GLOBALS
 */
    // This is the inter-task queue
volatile QueueHandle_t xQswP1a = NULL;
volatile QueueHandle_t xQswP1b = NULL;
volatile QueueHandle_t xQswP2 = NULL;
volatile QueueHandle_t xQswP3 = NULL;
volatile QueueHandle_t xQphrase = NULL;
volatile QueueHandle_t xQdot = NULL;
volatile QueueHandle_t xQt1 = NULL;


// FROM 1.0.1 Record references to the tasks
TaskHandle_t swP1_task_handle = NULL;
TaskHandle_t led1_task_handle = NULL;
TaskHandle_t latch_task_handle = NULL;
TaskHandle_t led2_task_handle = NULL;
TaskHandle_t gate1_task_handle = NULL;

    // These are the Send CW timers
volatile TimerHandle_t phrase_timer;
volatile TimerHandle_t dot_timer;
volatile TimerHandle_t t1_timer;


/*
 * FUNCTIONS
 */

/**
 * @brief Switch Debounce 
 * Repeat check of SW, send result to miso led pin task to stop and start blinking
 * Measures sw state, compares NOW state with PREVIOUS state. If states are different
 * sets count == 0 and looks for two states the same.  It then looks for five or more (MIN_COUNT)
 * in a row or more where NOW and PREVIOUS states are the same. Then Switch state is used
 * as a control signal, passed to an action function by a Queue.
*/ 

 void swP1_debounce(void* unused_arg) {
    uint8_t now = 1;            // initialize swP1_state
    uint8_t last = 1;    // initialize swP1_previous_state
    uint8_t count = 1;   // initialize swP1_final_state
    uint8_t pcfbuffer[2] = {0b11111111,0b11111111};
    
    while (true) {
        // Measure SW and add the LED state
        // to the FreeRTOS xQUEUE
        last = now; 
        i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, true);
        now = readBit(pcfbuffer[0], P05);
        if(last == now) {  // Just try again

            vTaskDelay(ms_delay100);  // check switch state every 20 ms
        }
        else if(last != now) {  // debounce new switch value
            count = 0;
            while(count < 3){
                last = now;
                i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, true);
                now = readBit(pcfbuffer[0], P05);
                if(last == now) { // switch has stopped bouncing
                    count++;
                }
                else{ // switch is still bouncing
                    count = 0;
                }
            vTaskDelay(ms_delay5);  // check switch state every 5 ms
            }
            xQueueSendToFront(xQswP1a, &now, 0);
            xQueueSendToFront(xQswP1b, &now, 0);
        }  // we only send switch change out, not continuous switch state
    }  // End while (true)    
}

/**
 * @brief Do something with the Switch Command
 * Wait for a Switch Change Queue Message
 * When a Switch change is received, Do something
 * Will respond to either HIGH or LOW Switch state
 */
 
 void led1_task(void* unused_arg) {
    uint8_t passed_value_buffer;
    uint8_t MsgWaitingkey;
    uint8_t sw_state;
    uint8_t pcfbuffer[2] = {0b11111111,0b11111111};
     
    while (true) {
        MsgWaitingkey = uxQueueMessagesWaiting(xQswP1a);
        if(MsgWaitingkey){ // Look for start Command with Switch 1
            xQueueReceive(xQswP1a, &passed_value_buffer,0);
            sw_state = passed_value_buffer;
            if(sw_state == 0) {  // do something
                printf(" LED1 task **1** SW1 state = %u  \n", sw_state);
           }
            else {  // do something else
                printf(" LED1 task **1** SW1 state = %u  \n", sw_state);
            }                  
        }
        vTaskDelay(ms_delay100);  // check switch state every 100 ms
    }  // End while (true)    
}

/**
 * @brief Latch Task
 * First debounce Switch then only output if there is a Switch DOWN output 
 * stop and start blinking
 * Toggle output at each Switch Down detected
 * 
 */

 void latch_task(void* unused_arg) {
    uint8_t now = 1;            // initialize sw1_state
    uint8_t last = 1;   // initialize sw1_previous_state
    uint8_t sw_state = 1;            // initialize sw1_state
    uint8_t sw_new = 1;            // initialize sw1_state
    uint8_t sw_old = 1;   // initialize sw1_previous_state
    uint8_t count = 1;   // initialize sw1_final_state
    uint8_t pcfbuffer[2] = {0b11111111,0b11111111};
    
//   0B0000|0000|0010|0001|0000|1000|0000|0000  16,11 
    uint32_t dot_mask = 0x00010800;
    uint32_t dot_LOW = 0x00000000;
    uint32_t dot_HIGH = 0x00010800; // two dots active
    
    while (true) {
        // Measure SW and add the LED state
        // to the FreeRTOS xQUEUE
        last = now;
        i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, true);
        now = readBit(pcfbuffer[0], P06);
        if(last == now) {  // Just try again

            vTaskDelay(ms_delay100);  // check switch state every 20 ms
        }
        else if(last != now) {  // debounce new switch value
            count = 0;
            while(count < 3){
                last = now;
                i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, true);
                now = readBit(pcfbuffer[0], P06);
                
                if(last == now) { // switch has stopped bouncing
                    count++;
                }
                else{  // switch is still bouncing
                    count = 0;
                }
            vTaskDelay(ms_delay5);  // check switch state every 5 ms
            }
    // Debounce is complete, now do the Latch output
            sw_state = now;

            if(sw_state == ON) {
                sw_new = !sw_old;  // Toggle action
                sw_old = sw_new;   // Save present state for next comparison
                if(sw_new == ON) {
//                    printf(" Do something \n");
//                  gpio_put_masked(dot_mask,dot_LOW);
//                    show_seven_seg_i2c(sw_new);  // turn on 0 symbol
                    xQueueSendToFront(xQswP2, &sw_new, 0);
                }
                else if(sw_new == OFF) {
//                    printf(" Turn it off \n");
//                    gpio_put_masked(dot_mask,dot_HIGH);
//                    show_seven_seg_i2c(20);  // turn off all LEDs
                    xQueueSendToFront(xQswP2, &sw_new, 0);
                }
            }
        }  // we only send switch change out, not continuous switch state
        vTaskDelay(ms_delay95);  // check switch state every 100 ms
    }  // End while (true)    
}


/**
 * @brief Do something with the Latch Switch Command
 * Wait for a Switch DOWN Queue Message
 * When a Switch DOWN is received, Do something
 */
 
 void led2_task(void* unused_arg) {
    uint8_t pcfbuffer[2] = {0b11111111,0b11111111};
    uint8_t passed_value_buffer;
    uint8_t MsgWaitingkey;
    uint8_t sw_state;
    uint32_t dot_timer_period = 1600;
    uint32_t phrase_timer_period = 2600;
    uint32_t period_sw_total;
    uint32_t period_sw_mask = 0x000E0000;
    uint32_t period_value_number;
    uint8_t delay_state;
    uint8_t DOTL_state;
    uint8_t DOTR_state;

    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op timer_info = {1,0};
    
    while (true) {
        period_sw_total = gpio_get_all(); // Read S1, S2, S3
        delay_state = (uint8_t)((period_sw_total &= period_sw_mask ) >>17); // Isolate SW values
        
        switch(delay_state) {   // Choose timer delay
            case 0:
                dot_timer_period = 200;
                break;
            case 1:
                dot_timer_period = 400;
                break;
            case 2:
                dot_timer_period = 600;
                break;
            case 3:
                dot_timer_period = 800;
                break;
            case 4:
                dot_timer_period = 1000;
                break;
            case 5:
                dot_timer_period = 1200;
                break;
            case 6:
                dot_timer_period = 1400;
                break;
            case 7:
                dot_timer_period = 1600;
                break;
            default:
                dot_timer_period = 1600;            
            } 
            
        MsgWaitingkey = uxQueueMessagesWaiting(xQswP2);  // Latch
        if(MsgWaitingkey){ // Look for DOWN Command with Switch 2
            xQueueReceive(xQswP2, &passed_value_buffer,0);
            sw_state = passed_value_buffer;
            show_seven_seg_i2c(delay_state);  // turn on 0 symbol
            
            if(sw_state == ON) { // DOWN Command
                gpio_put(DOTL, ON);
                // Timer Delays System ON command. 
                if (dot_timer != NULL) {  // First Timer to fire
                    xTimerChangePeriod(dot_timer, dot_timer_period, 0);  // set dot length, start timer
                     // xQueueReceive signals end of timed interval
                    xQueueReceive(xQdot, &timer_info, portMAX_DELAY); 
                    gpio_put(DOTL, OFF);

                    printf(" ** System ON ** %x\n",delay_state);               
                    gpio_put(DOTR, ON);  // show second dot
                    if (phrase_timer != NULL) { // Second timer fires at the time-out of First timer
                        xTimerChangePeriod(phrase_timer, phrase_timer_period, 0);  // set dot length, start timer
                        // xQueueReceive signals end of timed interval
                       xQueueReceive(xQphrase, &timer_info, portMAX_DELAY); 
                    gpio_put(DOTR, OFF);
                    }  
                }  
           }
            else {
                show_seven_seg_i2c(20);  // turn all LED OFF
                gpio_put(DOTR, OFF);
                gpio_put(DOTL, OFF);

                printf(" ** System OFF **\n");
            }                
        }
        vTaskDelay(ms_delay100);  // check switch state every 100 ms
    }  // End while (true)    
}

/**
 * @brief Do something with the Switch Command
 * Wait for a Switch Change Queue Message
 * When a Switch change is received, Do something
 * Switch DOWN fires first timer(DOTA on), then Second Timer(DOTA off)
 * 
 *                                DOTA on
 * ***************|_________________|***********************************
 *                    first timer              DOTA Off
 * *********************************|_____________|*********************
 *                                    second timer
 */
 
 void gate1_task(void* unused_arg) {
    uint8_t passed_value_buffer;
    uint8_t MsgWaitingkey;
    uint8_t sw_state;
    uint8_t pcfbuffer[2] = {0b11111111,0b11111111};
    uint32_t t1_timer_period1 = 2000;
    uint32_t t1_timer_period2 = 200;
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op timer_info = {3,0};
     
    while (true) {
        MsgWaitingkey = uxQueueMessagesWaiting(xQswP1b);
        if(MsgWaitingkey){ // Look for DOWN Command with Switch 1
            xQueueReceive(xQswP1b, &passed_value_buffer,0);
            sw_state = passed_value_buffer;
            if(sw_state == ON) {
                if (t1_timer != NULL) {  // First Timer to fire
                    xTimerChangePeriod(t1_timer, t1_timer_period1, 0);  // set dot length, start timer
                     // xQueueReceive signals end of timed interval
                    xQueueReceive(xQt1, &timer_info, portMAX_DELAY);
                    printf("Gate1 task **3a** if sw_state = %u  timer number %u\n", sw_state, timer_info.t_number);
                    gpio_put(DOTA, ON);  // lights at the end of First timeout
                }
                if (t1_timer != NULL) {  // next Timer to fire
                    xTimerChangePeriod(t1_timer, t1_timer_period2, 0);  // set dot length, start timer
                     // xQueueReceive signals end of timed interval
                    xQueueReceive(xQt1, &timer_info, portMAX_DELAY);
                    printf("Gate1 task **3b** if sw_state = %u  timer number %u\n", sw_state, timer_info.t_number);
                    gpio_put(DOTA, OFF);  // extinguises at the end of Second timeout
                    }

           }
            else {
                printf("Gate1 task **3** else sw_state = %u timer number %u\n",sw_state,timer_info.t_number);
            }                  
        }
        vTaskDelay(ms_delay75);  // check switch state every 75 ms
    }  // End while (true)    
}


/**
 * @brief Callback actioned when the CW timer fires.  Sends trigger to
 * initiate a new CW String TX.
 *
 * @param timer: The triggering timer.
 */
void phrase_timer_fired_callback(TimerHandle_t timer) {
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op timer_info = {1,0};

    if (timer == phrase_timer) {
        // The timer fired so trigger cw ID in led1 task
        printf(" ** System ON2 ** \n");               
        xQueueOverwrite(xQphrase, &timer_info);
       }
}

/**
 * @brief Callback actioned when the CW timer fires.  Sends trigger to
 * initiate a new CW String TX.
 *
 * @param timer: The triggering timer.
 */
void dot_timer_fired_callback(TimerHandle_t timer) {
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op timer_info = {2,0};

    if (timer == dot_timer) {
        // The timer fired so trigger cw ID in led2 task
        xQueueOverwrite(xQdot, &timer_info);
       }
}

/**
 * @brief Callback actioned when the CW timer fires.  Sends trigger to
 * initiate a new CW String TX.
 *
 * @param timer: The triggering timer.
 */
void t1_timer_fired_callback(TimerHandle_t timer) {
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op timer_info = {3,0};

    if (timer == t1_timer) {
        // The timer fired so trigger cw ID in led2 task
        xQueueOverwrite(xQt1, &timer_info); 
       }
}

/**
 * @brief Generate and print a debug message from a supplied string.
 *
 * @param msg: The base message to which `[DEBUG]` will be prefixed.
 */
void log_debug(const char* msg) {
    uint msg_length = 9 + strlen(msg);
    char* sprintf_buffer = malloc(msg_length);
    sprintf(sprintf_buffer, "[DEBUG] %s\n", msg);
//    #ifdef DEBUG
    printf("%s", sprintf_buffer);
//    #endif
    free(sprintf_buffer);
}


/*
 * RUNTIME START
 */
int main() {
    uint32_t error_state = 0;
    uint32_t pico_led_state = 0;
    uint32_t t1_period;
    struct op
    {
        uint32_t sw_number;
        uint32_t sw_state;
    }; 
    struct op sw_info = {0,0};
   
//   0B0000|0000|0011|0001|0011|1000|0000|0000  16,11,12,13,20,21 
    const uint32_t dot_mask = 0x00393800;
//   0B0000|0000|0000|1110|0000|0000|0000|0000  19,18,17 
    const uint32_t sw_mask = 0x000E0000;
//   0B0000|0000|0011|1111|0011|1000|0000|0000  11,12,13,16,17,18,19,20,21 
    const uint32_t init_mask = 0x003F3800;


    // Enable STDIO
    stdio_init_all();

    pcf8575_init();
    
    stdio_usb_init();
    // Pause to allow the USB path to initialize
    sleep_ms(2000);
    
     // Configure LED DOTS
    gpio_init_mask(init_mask);  // 11, 16 DOTL, DOTR
    gpio_set_dir_out_masked(dot_mask);

    gpio_disable_pulls(DOTL);  // remove pullup and pulldowns
    gpio_disable_pulls(DOTR);  // remove pullup and pulldowns
    gpio_disable_pulls(DOTA);  // remove pullup and pulldowns
    gpio_disable_pulls(DOTF);  // remove pullup and pulldowns
    gpio_disable_pulls(DOTD);  // remove pullup and pulldowns
    gpio_disable_pulls(DOTC);  // remove pullup and pulldowns
   
     // Configure SW0,SW1,SW2
    gpio_set_dir_in_masked(sw_mask);
    gpio_pull_up(SW0);  // pullup
    gpio_pull_up(SW1);  // pullup
    gpio_pull_up(SW2);  // pullup
   
    
    show_seven_seg_i2c(20);  // turn all LED OFF
    gpio_put(DOTR, 1);
    gpio_put(DOTL, 1);
    gpio_put(DOTA, 1);
    gpio_put(DOTF, 1);
    gpio_put(DOTD, 1);
    gpio_put(DOTC, 1);
                
   // label Program Screen
    printf("\x1B[2J");  // Clear Screen 
    printf("\x1B[%i;%iH",2,3);  // place curser
    printf("*** Switch Debounce and Latch *****");
    printf("\x1B[%i;%iH",4,3);  // place curser
    printf("**************************************");
    printf("\x1B[%i;%ir",6,15);  // set top and bottom lines of window

// Timer creates pause between repetition of the CW Text
    phrase_timer = xTimerCreate("PHRASE_TIMER", 
                            PAUSE_PERIOD,
                            pdFALSE,
                            (void*)PHRASE_TIMER_ID,
                            phrase_timer_fired_callback);
        if (phrase_timer == NULL) {
            error_state  += 1;
            }
            

// Timer creates dot length
    dot_timer = xTimerCreate("DOT_TIMER", 
                            DOT_PERIOD,
                            pdFALSE,
                            (void*)DOT_TIMER_ID,
                            dot_timer_fired_callback);
        if (dot_timer == NULL) {
            error_state  += 1;
            }
            
// Timer creates dot length
    t1_timer = xTimerCreate("T1_TIMER", 
                            t1_period,
                            pdFALSE,
                            (void*)T1_TIMER_ID,
                            t1_timer_fired_callback);
        if (t1_timer == NULL) {
            error_state  += 1;
            }
            
    // Set up tasks
    // FROM 1.0.1 Store handles referencing the tasks; get return values
    // NOTE Arg 3 is the stack depth -- in words, not bytes
    BaseType_t swP1_status = xTaskCreate(swP1_debounce, 
                                         "SW1_TASK", 
                                         256, 
                                         NULL, 
                                         8,     // Task priority
                                         &swP1_task_handle);
        if (swP1_status != pdPASS) {
           error_state  += 1;
            }

      BaseType_t  led1_status = xTaskCreate(led1_task, 
                                         "LED1_TASK", 
                                         256, 
                                         NULL, 
                                         6,     // Task priority
                                         &led1_task_handle);
        if (led1_status != pdPASS) {
           error_state  += 1;
            }
           
     BaseType_t  latch_status = xTaskCreate(latch_task, 
                                         "LATCH_TASK", 
                                         256, 
                                         NULL, 
                                         7,     // Task priority
                                         &latch_task_handle);
        if (latch_status != pdPASS) {
           error_state  += 1;
            }
            
      BaseType_t  led2_status = xTaskCreate(led2_task, 
                                         "LED2_TASK", 
                                         256, 
                                         NULL, 
                                         5,     // Task priority
                                         &led2_task_handle);
        if (led2_status != pdPASS) {
           error_state  += 1;
            }
           
      BaseType_t  gate1_status = xTaskCreate(gate1_task, 
                                         "GATE1_TASK", 
                                         256, 
                                         NULL, 
                                         4,     // Task priority
                                         &gate1_task_handle);
        if (gate1_status != pdPASS) {
           error_state  += 1;
            }
           
             
  
   // Set up the event queue
    xQswP1a = xQueueCreate(1, sizeof(uint8_t)); 
    if (xQswP1a == NULL) error_state += 1;

    xQswP1b = xQueueCreate(1, sizeof(uint8_t)); 
    if (xQswP1b == NULL) error_state += 1;

    xQswP2 = xQueueCreate(1, sizeof(uint8_t)); 
    if (xQswP2 == NULL) error_state += 1;

    xQt1= xQueueCreate(1, sizeof(uint8_t)); 
    if (xQt1 == NULL) error_state += 1;

    xQphrase = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQphrase == NULL ) error_state += 1;

    xQdot = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQdot == NULL ) error_state += 1;


    // Start the FreeRTOS scheduler
    // FROM 1.0.1: Only proceed if no tasks signal error in setup
    if (error_state == 0) {
        vTaskStartScheduler();
    }
    else {   // if tasks don't initialize, light pico board led
    // Configure PICO_LED_PIN for Initialization failure warning
        gpio_init(PICO_LED_PIN);
        gpio_disable_pulls(PICO_LED_PIN);  // remove pullup and pulldowns
        gpio_set_dir(PICO_LED_PIN, GPIO_OUT);
   
        pico_led_state = 1;
        gpio_put(PICO_LED_PIN, pico_led_state);
        while (true);
    }
    
    // We should never get here, but just in case...
    while(true) {
        // NOP
    };
}
