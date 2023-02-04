/**
 * RP2040 FreeRTOS Text Buffer 
 * 
 * @copyright 2022, Tony Smith (@smittytone)
 * @version   1.2.0
 * @licence   MIT
 *
 */
#include <stdio.h>
#include "main.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "../Common/seven_seg.h"
#include "../Common/pcf8575i2c.h"
#include "../Common/ps2.h"

#define ASCII_ESC 27
#define I2C_ADDR 0x20

/*
 * GLOBALS
 */
// This is the inter-task queue
// volatile QueueHandle_t qpinb = NULL;
volatile QueueHandle_t qpinf = NULL;
volatile QueueHandle_t qbuffer = NULL;
volatile QueueHandle_t qtxtctl = NULL;

// FROM 1.0.1 Record references to the tasks
TaskHandle_t pinb_led_task_handle = NULL;
TaskHandle_t pinf_led_task_handle = NULL;
TaskHandle_t txtbuffer_task_handle = NULL;
TaskHandle_t on_off_task_handle = NULL;


/*
 * FUNCTIONS
 */

/**
 * @brief Repeat check of txtctl, send result to miso led pin task to 
 * stop and start blinking
 * Waits for text input (on/off). Sets segments D and G appropriately
 * Sends flag to pinf_led_task to command blink 
 */
void on_off_task(void* unused_arg) {
    uint32_t flag = 1; 
    uint32_t command_string_length = 0;
    uint8_t i = 0; //  counter
    uint8_t pcfbuffer[2] = {0b11111111,0b11111111};
    uint32_t on_state = 1;    // flag to indicate "on" found
    uint32_t off_state = 1;   // flag to indicate "off" found
    char ON[] = "ON";         // reference string 
    char OFF[] = "OFF";       // reference string 
    char buffer_size = 64; 
    BaseType_t xStatus;
     
    while (true) {
        char* control_string = malloc(buffer_size);
        for(i=0;i<buffer_size; i++) {
            control_string[i] = '\0'; // fill buffer with string end char
        }
        xStatus = xQueueReceive(qbuffer,control_string, portMAX_DELAY);
        if( xStatus == pdPASS ) { 
            command_string_length = strlen(control_string);
            // Queue definition sets control_string queue to 64 length but control word
            // is shortened and cut at the CR to make the shorter string.
            switch(command_string_length) {
                case 0:
                case 1:
                    break;
                case 2:
                    on_state  = memcmp(ON,control_string ,2);
                    if (on_state == 0 ){  // on 
                       i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, true);
                        pcfbuffer[1] = setBit_High(pcfbuffer[1],6); 
                        pcfbuffer[1] = setBit_Low(pcfbuffer[1],0); 
                        i2c_write_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, true);
                        flag = 1;
                        xQueueOverwrite(qtxtctl, &flag);  // send command to pinf_led_task
                        }
                    break;
                case 3:
                    off_state = memcmp(OFF,control_string,3);
                    if (off_state == 0 ){  // off
                        pcfbuffer[1] = 0b11111111;
                        pcfbuffer[1] = setBit_Low(pcfbuffer[1],6); 
                        i2c_write_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, true);
                        flag = 0;
                        xQueueOverwrite(qtxtctl, &flag);  // send command to pinf_led_task
                        }
                    break;
                default:
                    break;
                }
            }
        else
            {
            log_debug("\x1B[12;3H\x1B[2K");  // place curser, clear rest of line
            log_debug("Could not receive from the queue.\r\n");
            log_debug("\x1B[7;3H\x1B[2K");   // place curser, clear rest of line
            log_debug("Enter on/off: ");     // prompt
            }            

        on_state  = 1;
        off_state = 1;
        free(control_string);
        vTaskDelay(ms_delay300);  // check text entry state every 300 ms
   }  // End while (true)   
}
/**
 * @brief Repeatedly flash Pin F LED.
 */
void pinf_led_task(void* unused_arg) {
    uint8_t pinf_led_state = 0;
    uint32_t qtxt_buffer = 0;
    uint8_t pcfbuffer[2] = {0b11111111,0b11111111};
    
    while (true) {
        // Turn Pico LED on and add the LED state
        // to the FreeRTOS xQUEUE
        xQueuePeek(qtxtctl, &qtxt_buffer, portMAX_DELAY); 
        if (qtxt_buffer == 1) {
            pinf_led_state = 0;
              /* ******************************* */
            i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, true);
            pcfbuffer[1] = setBit_Low(pcfbuffer[1],2); 
            pcfbuffer[1] = setBit_High(pcfbuffer[1],4);
            i2c_write_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, true);
            xQueueSendToFront(qpinf, &pinf_led_state, 0);
            vTaskDelay(ms_delay200);
        }   
        else { 
            vTaskDelay(ms_delay20);
        }  // End if passed_value_buffer
        
        xQueuePeek(qtxtctl, &qtxt_buffer, portMAX_DELAY); 
        if (qtxt_buffer == 1) {
        
            pinf_led_state = 1;
            i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, true);
            pcfbuffer[1] = setBit_Low(pcfbuffer[1],4); 
            pcfbuffer[1] = setBit_High(pcfbuffer[1],2);
            i2c_write_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, true);
            xQueueSendToFront(qpinf, &pinf_led_state, 0);
            vTaskDelay(ms_delay200);
            }
        else {  
            vTaskDelay(ms_delay20);
        }  // End if passed_value_buffer
    }
}

/**
 * @brief Repeatedly flash an LED connected to LED pin B
 *        based on the value passed via the inter-task queue.
 *        LED pin B will flash opposite to LED F of pinf_led_task
 */
void pinb_led_task(void* unused_arg) {
    uint8_t qpinf_buffer  = 0;
    
    while (true) {
        // Check for an item in the FreeRTOS xQueue
        if (xQueueReceive(qpinf, &qpinf_buffer, portMAX_DELAY) == pdPASS) {
            // Received a value so flash the GPIO LED accordingly
            // (NOT the sent value)
        gpio_put(DOTL, qpinf_buffer == 1 ? 0 : 1);
        gpio_put(DOTR, qpinf_buffer == 1 ? 1 : 0);
        }
        else {
            vTaskDelay(ms_delay20); 
        }            
    }
}

/**
 * @brief Repeat check of txtctl, send result to miso led pin task to 
 * stop and start blinking
 * Waits for text input (YES/no). If "YES" blink. IF "no" stop blinking.
 * Sends command to miso_led_task with queue mailbox (qmi)
 * as a control signal.
 */
void txt_buffer_task(void* unused_arg) {
    uint8_t i = 0; //  counter
    uint8_t buffer_size = 64; 
    BaseType_t xStatus;
    char new_char = 'A';
    
    while (true) {
        char* control_string = malloc(buffer_size);
        for(i=0;i<buffer_size; i++) {
            control_string[i] = '\0'; // fill buffer with string end char
        }
        log_debug("\x1B[7;3H\x1B[2KEnter on/off: ");  // place curser, clear rest of line
        for(i=0;i<buffer_size; i++) { // Enter characters till CR finishes
            new_char = toupper(get_iso8859_code());
            if((new_char == 0x0D) || (i > buffer_size - 1)) {
                break;
            }
            control_string[i] = new_char;
            vTaskDelay(ms_delay173);  // check text entry state every 173 ms
        }
        xStatus = xQueueSendToFront(qbuffer,control_string, 0);
        if( xStatus != pdPASS )  // show warning, reset input prompt
            {
                log_debug("\x1B[12;3H\x1B[2K");  // place curser, clear rest of line
                log_debug( "Could not send to the queue.\r\n" );
                log_debug("\x1B[7;3H\x1B[2K");  // place curser, clear rest of line
                log_debug("Enter on/off: ");          // prompt
            }
        free(control_string);
     }  // End while (true)    
} 

/**
 * @brief Generate and print a debug message from a supplied string.
 *
 * @param msg: The base message to which `[DEBUG]` will be prefixed.
 */
void log_debug(const char* msg) {
    uint msg_length = strlen(msg);
    char* sprintf_buffer = malloc(msg_length);
    sprintf(sprintf_buffer, "%s", msg);
    printf("%s",sprintf_buffer);
    free(sprintf_buffer);
}

/**
 * @brief Show basic device info. 
 */
void log_device_info(void) {
    printf("\x1B[2J");
    printf("\x1B[%i;%iH", 2,3);
    printf("App Version: %s", APP_VERSION); 
    printf("\x1B[%i;%iH", 4,3);
    printf("Build: %i", BUILD_NUM);
    printf("\x1B[%i;%iH", 6,3);
}

/*
 * RUNTIME START
 */
int main() {
    uint8_t pico_led_state = 0;  // signal startup failure
    uint8_t error_state = 0;     // startup failure count
    char buffer_size = 64;    // flag to send "yes"

    // Enable STDIO
    stdio_init_all();

    stdio_usb_init();
    // Pause to allow the USB path to initialize
    sleep_ms(2000);
    
    // Initialize port extender
    pcf8575_init();
    
    // Configure the PINF LED
    gpio_init(DOTL);
    gpio_set_dir(DOTL, GPIO_OUT);
    gpio_disable_pulls(DOTL);  // remove pullup and pulldowns

    // Configure the PINF LED
    gpio_init(DOTR);
    gpio_set_dir(DOTR, GPIO_OUT);
    gpio_disable_pulls(DOTR);  // remove pullup and pulldowns

        // label Program Screen
    printf("%c[2J", ASCII_ESC);  // Clear Screen
    printf("%c[%i;%iH", ASCII_ESC,2,3);  // place curser
    printf("*** Text Control Program ***");
    printf("\x1B[%i;%iH",4,2);  // place curser
    printf("**************************************\n");

    printf("\x1B[6;8r"); // set window lines 6;8
    printf("\x1B[%i;%iH**********************",6,2);  // place curser
    printf("\x1B[%i;%iH**********************",8,2);  // place curser
    printf("\x1B[%i;%iH\x1B[2K",7,3);  // place curser, clear rest of line
    printf("Enter on/off: ");  // prompt
    
     // Set up three tasks
    // FROM 1.0.1 Store handles referencing the tasks; get return values
    // NOTE Arg 3 is the stack depth -- in words, not bytes
    BaseType_t pinf_status = xTaskCreate(pinf_led_task, 
                                         "PINF_LED_TASK", 
                                         256, 
                                         NULL, 
                                         7, 
                                         &pinf_led_task_handle);
        if (pinf_status != pdPASS) {
            error_state += 1;
            }

    BaseType_t pinb_status = xTaskCreate(pinb_led_task, 
                                         "PINB_LED_TASK", 
                                         256, 
                                         NULL, 
                                         6, 
                                         &pinb_led_task_handle);
        if (pinb_status != pdPASS) {
            error_state += 1;
            } 
    
    BaseType_t txtbuffer_status = xTaskCreate(txt_buffer_task, 
                                         "TXT_BUFFER_TASK", 
                                         256, 
                                          NULL, 
                                         4,     // Task priority
                                         &txtbuffer_task_handle);
        if (txtbuffer_status != pdPASS) {
            error_state += 1;
            }

    BaseType_t on_off_status = xTaskCreate(on_off_task, 
                                         "ON_OFF_TASK", 
                                         256, 
                                          NULL, 
                                         5,     // Task priority
                                         &on_off_task_handle);
        if (on_off_status != pdPASS) {
            error_state += 1;
            }

   // Set up the event queue
    qpinf = xQueueCreate(1, sizeof(uint8_t));
      if ( qpinf == NULL ) error_state += 1;
    qbuffer = xQueueCreate(1,buffer_size);
      if ( qbuffer == NULL ) error_state += 1;
    qtxtctl = xQueueCreate(1, sizeof(uint32_t));
      if ( qtxtctl == NULL ) error_state += 1;
    
    // Start the FreeRTOS scheduler
    // FROM 1.0.1: Only proceed with valid tasks
    if (error_state == 0) {
        vTaskStartScheduler();
    }
    else {   // if tasks don't initialize, light pico board led
     // Configure the PINF LED
            gpio_init(PICO_LED_PIN);
            gpio_set_dir(PICO_LED_PIN, GPIO_OUT);
            gpio_disable_pulls(PICO_LED_PIN);  // remove pullup and pulldowns
 
            pico_led_state = 1;
            gpio_put(PICO_LED_PIN, pico_led_state);
     }
    
    // We should never get here, but just in case...
    while(true) {
        // NOP
    };
}

 

