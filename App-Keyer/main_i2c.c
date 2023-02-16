/**
 * RP2040 FreeRTOS Template: Send CW with Timer trigger
 * Timer is monostable, creating delay between text sent
 * Also learning to use structures in Queue
 * Sending Morse Code text
 * Text selected by value read in by ADC
 * 
 * built on 
 * @copyright 2022, Tony Smith (@smittytone)
 * @version   1.2.0
 * @licence   MIT
 *
 */
#include <stdio.h>
#include "main.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "../Common/seven_seg.h"
#include "../Common/pcf8575i2c.h"

#define INTERVALS 6plin
#define PAUSE_PERIOD 2800
#define DOT_PERIOD 75
#define SENTENCE 1
#define I2C_ADDR 0x20



/* 
 * GLOBALS
 */
    // FROM 1.0.1 Record references to the tasks
TaskHandle_t cw_task_handle = NULL;
TaskHandle_t select_phrase_task_handle = NULL;

    // These are the Send CW timers
volatile TimerHandle_t cw1_timer;
volatile TimerHandle_t cw2_timer;

    // These are the inter-task queues
volatile QueueHandle_t xQtimer1 = NULL;
volatile QueueHandle_t xQtimer2 = NULL;
volatile QueueHandle_t xQueue3 = NULL;
volatile QueueHandle_t xQueue4 = NULL;
volatile QueueHandle_t xQphrase5 = NULL;



/*
 * FUNCTIONS
 */


/**
 * @brief Send CW, blinking RX LED 
 *        xQtimer1 triggers TX of phrase chosen by ADC input value
 */
void cw_task(void* unused_arg) {

    uint32_t i  = 1;
    uint32_t number  = 1;
    uint32_t phrase_select  = 1;
    char this_char = 0;
    char last_char = 0;
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op t1_info = {1,1};
     
    // Selection of Texts
    char **pstrings;
    char *pnext_string;
    
    char pfox[] = "THE QUICK BROWN FOX JUMPED OVER THE LAZY DOGS BACK 0123456789\0";
    char pabc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\0";
    char pnumbers[] = "0123456789\0";
    char paris[] = "PARIS PARIS PARIS PARIS PARIS\0";
    char pve[]  = "VE7BGV\0";
    char pvel[] = "ve7bgv\0";
    char pcq[] = "CQ CQ CQ DE VE7BGV CQ CQ CQ DE VE7BGV K\0";
    char pph[] = "CQ CQ CQ DE VE7BGV\0";
    char *phrases[9] = {pfox,pabc,pnumbers,paris,pve,pvel,pcq,pph,0x0000};
    pstrings = phrases;  // transmit just one string pointed to by ptr
    
    vTaskDelay(ms_delay300);
    uint32_t first_flag  = 1;
    uint32_t timer_flag  = 0;
    
    
   while (true) {
         if(timer_flag == 0) {  // Timer has triggered timeout
            xQueuePeek(xQphrase5, &phrase_select, portMAX_DELAY); 
            pnext_string = phrases[phrase_select];  // select string to be TX
            i = 0;  // sets up start of TX phrase
            last_char = 0x40;  // dummy value
            while (pnext_string[i] != 0) {   // step through TX phrase
                this_char = pnext_string[i]; 
                printf("%c",this_char);
                // ********************************************************
                // Correct between-word SPACE length. Each character ends
                // with three letter SPACES (a between-letter SPACE) The
                // first between-word SPACE needs four more between-letter
                // SPACES, all SPACES after the first SPACE needs seven SPACES.
                // so they need a new output character.   
                if((this_char == 0x20) && (last_char == 0x20)) {
                    this_char = 0x21;  // correct SPACE Length
                }
                else if((this_char == 0x20) && (last_char == 0x21)) {
                    this_char = 0x21;  // correct SPACE Length
                }
                       // ********************************************************
                          
                send_CW(this_char);
                i += 1;  // point to next character
                last_char = this_char;  // to correct "short" SPACE Char
            }  // end of while(next_string[i] ...
            printf("\n");
     
                // Start the monostable timer
        } // end of if(timer_flag == 0) ...
        // Trigger one-shot timer to delay next CW text output
        if (cw1_timer != NULL) xTimerStart(cw1_timer, 0);
        xQueueReceive(xQtimer1, &t1_info, portMAX_DELAY); // xQueueReceive empties queue
        timer_flag = t1_info.t_state;
        t1_info.t_state = 1;
    }  // end of while(true)
}

/**
 * @brief CW print a Morse character from a supplied ASCII character.
 * Morse char 'F' is ". . - ." converted to 0x1175 or Binary 1000101110101
 * This Binary number is shifted RIGHT one bit at a time.  Each Morse letter ends
 * with three '0's (making one space following the letter). The leftmost '1' is
 * the flag to indicate that the letter is completely sent.
 *
 */
void send_CW(char ascii_in) {
    uint32_t masknumber = 0x00000001;
    uint8_t this_char = 0;
    uint32_t bit_out = 1;
    uint32_t morse_out = 0;
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op t1_info = {1,1};

    
// *********************************************************************************************     
//                        A      B      C     D    E      F      G     H     I     J
    uint32_t morse[] = {0x11D,0x1157,0x45D7,0x457,0x11,0x1175,0x1177,0x455,0x45,0x11DDD,
//        K      L     M     N      O      P      Q      R     S    T    U      V      W
       0X11D7,0X115D,0X477,0X117,0X4777,0X22DD,0X11D77,0X45D,0X115,0X47,0X475,0X11D5,0X11DD,
//      X       Y       Z       0         1        2       3       4     5      6 
       0X4757,0X11DD7,0X4577,0X477777,0x11DDDD,0X47775,0X11DD5,0X4755,0X1155,0X4557,
//       7        8         9     SP  SPlg
       0X11577,0X45777,0X117777,0X10,0X80};
// *********************************************************************************************     
        
        this_char = ascii_in;
        if (this_char == 32) { // look for SPACE char
            this_char = 36;    // index to Morse char
            }
        else if (this_char == 33) { // look for SPACE char
            this_char = 37;    // index to Morse char
            }
        else if (isdigit(this_char)) {    // look for Numbers
            this_char = this_char - 22 ;  // index to Morse char
            }
        else if(islower(this_char)) {     // look for Lower Case
            this_char = toupper(this_char);
            this_char = this_char - 65;   // index to Morse char
            }
        else if(isalpha(this_char)){
            this_char = this_char - 65;   // index to Morse char
            }
        else  {
            this_char = 36; 
            }      
        morse_out = morse[this_char];     // get Morse char from array
        
    while (morse_out != 1) {              // send Morse bits              
        bit_out = (morse_out & masknumber);  // isolate bit to send
        gpio_put(DOTR, !bit_out);  //  Dots need LOW to turn ON  
        gpio_put(DOTL, !bit_out); // 
        
        morse_out /= 2;                   // divide by 2 to shift Right
        if (cw2_timer != NULL) xTimerStart(cw2_timer, 0);  // define dot length 
        xQueueReceive(xQtimer2, &t1_info, portMAX_DELAY); // xQueueReceive empties queue
        }

} 


/**
 * @brief Callback actioned when the CW timer fires.  Sends trigger to
 * initiate a new CW String TX.
 *
 * @param timer: The triggering timer.
 */
void cw1_timer_fired_callback(TimerHandle_t timer) {
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op timer_info = {1,0};

    if (timer == cw1_timer) {
        // The timer fired so trigger cw ID in cw task
        xQueueOverwrite(xQtimer1, &timer_info);
       }
}


/**
 * @brief Callback actioned when the CW timer fires.  Sends trigger to
 * initiate a new CW String TX.
 *
 * @param timer: The triggering timer.
 */
void cw2_timer_fired_callback(TimerHandle_t timer) {
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op timer_info = {2,0};

    if (timer == cw2_timer) {
        // The timer fired so trigger cw ID in cw task
        xQueueOverwrite(xQtimer2, &timer_info);
       }
}

/* 
 * @brief Read SW1, SW2 and SW3, calculate binary value, 
 * as a control signal, passed to an action function by a Queue. 
 * ADC reading divided into segments. Value of segments displayed on
 * a Seven Segment LED module
 */

 void select_phrase_task(void* unused_arg) {  // selects which Text phrase to o/p

    const uint8_t  sw_input_mask = 0b00001111;
    uint32_t select_val;
    uint32_t temp_sw_input;
    uint8_t  pcfbuffer[2]={0b11111111,0b11111111};// data buffer, must be two bytes
   
    while (true) {
        // Read Switches on i2c ports 0, 1, and 2 and add the LED state
        // to the FreeRTOS xQUEUE
      i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, false);
      temp_sw_input = pcfbuffer[0]; // protect input values from change
      select_val = (temp_sw_input &= sw_input_mask);
      show_seven_seg_i2c(select_val);
    
      xQueueOverwrite(xQphrase5, &select_val);  // ADC output refresh
      vTaskDelay(ms_delay300);  // check adc value every 300 ms
    }  // End while (true)    
}


/**
 * @brief Generate and print a message from a supplied string.
 * Curser is set before this function call. This function does not 
 * provide CR.
 *
 */
 /*
void print_msg(const char* msg) {
    uint msg_length = strlen(msg);
    char* sprintf_buffer = malloc(msg_length);
    sprintf(sprintf_buffer, "%s\n", msg);
    printf("%s", sprintf_buffer);
    free(sprintf_buffer);
}
*/

/**
 * @brief Initialize GPIO Pin for input and output.
 *
 */
void configure_gpio(void) {
    uint32_t pico_led_state = 0;

    // Enable STDIO
    stdio_init_all();

    stdio_usb_init();
    // Pause to allow the USB path to initialize
    sleep_ms(2000);
    
    // Configure PICO_LED_PIN for Initialization failure warning
    gpio_init(PICO_LED_PIN);
    gpio_disable_pulls(PICO_LED_PIN);  // remove pullup and pulldowns
    gpio_set_dir(PICO_LED_PIN, GPIO_OUT);

    // Configure PICO_LED_PIN for Initialization failure warning
    gpio_init(DOTL);
    gpio_disable_pulls(DOTL);  // remove pullup and pulldowns
    gpio_set_dir(DOTL, GPIO_OUT);

    // Configure PICO_LED_PIN for Initialization failure warning
    gpio_init(DOTR);
    gpio_disable_pulls(DOTR);  // remove pullup and pulldowns
    gpio_set_dir(DOTR, GPIO_OUT);

    // Configure Seven Segment LED with two dots
    config_seven_seg();
    
    // Enable board LED
    gpio_put(PICO_LED_PIN, pico_led_state);  // set initial state to OFF
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

    pcf8575_init();
    configure_gpio();
    
    // label Program Screen
    printf("\x1B[2J");  // Clear Screen
    printf("\x1B[%i;%iH",2,3);  // place curser
    puts("*** CW Sending  Program ***");
    printf("\x1B[%i;%iH",4,3);  // place curser
    puts("**************************************");
    printf("\x1B[%i;%ir",6,15);  // set window top and bottom lines
 
// Timer creates pause between repetition of the CW Text
    cw1_timer = xTimerCreate("CW1_TIMER", 
                            PAUSE_PERIOD,
                            pdFALSE,
                            (void*)PHRASE_TIMER_ID,
                            cw1_timer_fired_callback);
        if (cw1_timer == NULL) {
            error_state  += 1;
            }
            

// Timer creates dot length
    cw2_timer = xTimerCreate("CW2_TIMER", 
                            DOT_PERIOD,
                            pdFALSE,
                            (void*)DIT_TIMER_ID,
                            cw2_timer_fired_callback);
        if (cw2_timer == NULL) {
            error_state  += 1;
            }
            

    // Set up tasks
    // FROM 1.0.1 Store handles referencing the tasks; get return values
    // NOTE Arg 3 is the stack depth -- in words, not bytes

    BaseType_t cw_status = xTaskCreate(cw_task, 
                                         "CW_LED_PIN_TASK", 
                                         256, 
                                         NULL, 
                                         5, 
                                         &cw_task_handle);
        if (cw_status != pdPASS) {
            error_state  += 1;
            }
            
    BaseType_t select_phrase_task_status = xTaskCreate(select_phrase_task, 
             
                                         "SELECT_PHRASE_TASK", 
                                         256, 
                                         NULL, 
                                         6,     // Task priority
                                         &select_phrase_task_handle);
        if (select_phrase_task_status != pdPASS) {
            error_state  += 1;
            }
             
   // Set up the event queue
    xQtimer1 = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQtimer1 == NULL ) error_state += 1;

    xQtimer2 = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQtimer2 == NULL ) error_state += 1;

    xQueue3 = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQueue3 == NULL ) error_state += 1; 

    xQueue4 = xQueueCreate(1, sizeof(sw_info));
    if ( xQueue4 == NULL ) error_state += 1;
     
    xQphrase5 = xQueueCreate(1, sizeof(uint32_t));
    if ( xQphrase5 == NULL ) error_state += 1;
    
    // Start the FreeRTOS scheduler 
    // FROM 1.0.1: Only proceed if no tasks signal error in setup
    if (error_state == 0) {
        vTaskStartScheduler();
    }
    else {   // if tasks don't initialize, pico board led will light   
        pico_led_state = 1;
        gpio_put(PICO_LED_PIN, pico_led_state);
    }
    
    // We should never get here, but just in case...
    while(true) {
        // NOP
    };
}
