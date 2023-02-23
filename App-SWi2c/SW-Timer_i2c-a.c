/**
 * RP2040 FreeRTOS Shuffle
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
volatile QueueHandle_t xQswP1 = NULL;
volatile QueueHandle_t xQswP2 = NULL;
volatile QueueHandle_t xQswP3 = NULL;
volatile QueueHandle_t xQphrase = NULL;
volatile QueueHandle_t xQdot = NULL;


// FROM 1.0.1 Record references to the tasks
TaskHandle_t swP1_task_handle = NULL;
TaskHandle_t led1_task_handle = NULL;
TaskHandle_t latch_task_handle = NULL;
TaskHandle_t led2_task_handle = NULL;

    // These are the Send CW timers
volatile TimerHandle_t phrase_timer;
volatile TimerHandle_t dot_timer;


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
    uint8_t last = 1;   // initialize swP1_previous_state
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
            while(count < 4){
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
            xQueueSendToFront(xQswP1, &now, 0);
        }  // we only send switch change out, not continuous switch state
    }  // End while (true)    
}

/**
 * @brief Do something with the Switch Command
 * Wait for a Switch Change Queue Message
 * When a Switch change is received, Do something
 */
 
 void led1_task(void* unused_arg) {
    uint8_t passed_value_buffer;
    uint8_t MsgWaitingkey;
    uint8_t sw_state;
    uint8_t pcfbuffer[2] = {0b11111111,0b11111111};
     
    while (true) {
        MsgWaitingkey = uxQueueMessagesWaiting(xQswP1);
        if(MsgWaitingkey){ // Look for start Command with Switch 1
            xQueueReceive(xQswP1, &passed_value_buffer,0);
            sw_state = passed_value_buffer;
            if(sw_state == 0) {
//                show_seven_seg_i2c(1);
                printf(" **1** sw_state = %u  \n", sw_state);
           }
            else {
//                show_seven_seg_i2c(0);
                printf(" **1** sw_state = %u  \n", sw_state);
            }                  
        }
        vTaskDelay(ms_delay100);  // check switch state every 100 ms
    }  // End while (true)    
}

/**
 * @brief Latch Task
 * First debounce Switch then only output if there is a Switch Down output 
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
            while(count < 4){
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

            if(sw_state == 0) {
                sw_new = !sw_old;  // Toggle action
                sw_old = sw_new;   // Save present state for next comparison
                if(sw_new == 0) {
//                    printf(" Do something \n");
                    gpio_put(DOTR, sw_new);  // F2
                    gpio_put(DOTL, sw_new); // G11
//                    show_seven_seg_i2c(sw_new);  // turn on 0 symbol
                    xQueueSendToFront(xQswP2, &sw_new, 0);
                }
                else if(sw_new == 1) {
//                    printf(" Turn it off \n");
                    gpio_put(DOTR, sw_new);  // F2
                    gpio_put(DOTL, sw_new); // G11
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
 * Wait for a Switch Change Queue Message
 * When a Switch change is received, Do something
 */
 
 void led2_task(void* unused_arg) {
    uint8_t pcfbuffer[2] = {0b11111111,0b11111111};
    uint8_t passed_value_buffer;
    uint8_t MsgWaitingkey;
    uint8_t sw_state;
    uint32_t dot_timer_period = 1600;
    uint32_t period_sw_total;
    uint32_t period_sw_mask = 0x000E0000;
    uint32_t period_value_number;
    uint8_t delay_state;

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
            
        MsgWaitingkey = uxQueueMessagesWaiting(xQswP2);
        if(MsgWaitingkey){ // Look for start Command with Switch 1
            xQueueReceive(xQswP2, &passed_value_buffer,0);
            sw_state = passed_value_buffer;
            
            if(sw_state == 0) {
                // Timer Delays System ON command. 
                if (dot_timer != NULL) {
                    xTimerChangePeriod(dot_timer, dot_timer_period, 0);  // set dot length, start timer
                     // xQueueReceive signals end of timed interval
                    xQueueReceive(xQdot, &timer_info, portMAX_DELAY); 
                    printf(" ** System ON ** %x\n",delay_state);               
                    show_seven_seg_i2c(delay_state);
                }  
           }
            else {
                show_seven_seg_i2c(20);  // turn all LED OFF
                printf(" ** System OFF **\n");
            }                
        }
        vTaskDelay(ms_delay100);  // check switch state every 100 ms
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
    struct op
    {
        uint32_t sw_number;
        uint32_t sw_state;
    }; 
    struct op sw_info = {0,0};


    // Enable STDIO
    stdio_init_all();

    pcf8575_init();
    
    stdio_usb_init();
    // Pause to allow the USB path to initialize
    sleep_ms(2000);
    
     // Configure LED DOTS
    gpio_init(DOTL);
    gpio_disable_pulls(DOTL);  // remove pullup and pulldowns
    gpio_set_dir(DOTL, GPIO_OUT);
   
    // Configure LED DOTS
    gpio_init(DOTR);
    gpio_disable_pulls(DOTR);  // remove pullup and pulldowns
    gpio_set_dir(DOTR, GPIO_OUT);
   
     // Configure SW0
    gpio_init(SW0);
    gpio_pull_up(SW0);  // pullup
    gpio_set_dir(SW0, GPIO_IN);
   
     // Configure SW1
    gpio_init(SW1);
    gpio_pull_up(SW1);  // pullup
    gpio_set_dir(SW1, GPIO_IN);
   
     // Configure SW2
    gpio_init(SW2);
    gpio_pull_up(SW2);  // pullup
    gpio_set_dir(SW2,  GPIO_IN);
    
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
            
    // Set up tasks
    // FROM 1.0.1 Store handles referencing the tasks; get return values
    // NOTE Arg 3 is the stack depth -- in words, not bytes
    BaseType_t swP1_status = xTaskCreate(swP1_debounce, 
                                         "SW1_TASK", 
                                         256, 
                                         NULL, 
                                         6,     // Task priority
                                         &swP1_task_handle);
        if (swP1_status != pdPASS) {
           error_state  += 1;
            }

      BaseType_t  led1_status = xTaskCreate(led1_task, 
                                         "LED1_TASK", 
                                         256, 
                                         NULL, 
                                         4,     // Task priority
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
           
             
  
   // Set up the event queue
    xQswP1 = xQueueCreate(1, sizeof(uint8_t)); 
    if (xQswP1 == NULL) error_state += 1;

    xQswP2 = xQueueCreate(1, sizeof(uint8_t)); 
    if (xQswP2 == NULL) error_state += 1;

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
