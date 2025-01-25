 /**
 * RP2040 FreeRTOS Template: Send CW with Timer trigger
 * Timer is monostable, creating delay between text sent
 * Also learning to use structures in Queue
 * Sending Morse Code text
 * Text selected by value read in by three switches on pcf8575 P0, P1, P2
 * One text phrase may be entered into phrase No. 7 through the PS2 Keyboard
 * CR commands text to be passed by Queue to CW task and entered in pph phrase
 * The added Text can then be selected by the switches and sent as CW
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
#include "../Common/ps2.h"

#define INTERVALS 6plin
#define PAUSE_PERIOD 2800
#define DOT_PERIOD 75
#define SENTENCE 1
#define I2C_ADDR 0x20
#define RING_SIZE 64



/*  
 * GLOBALS
 */
    // FROM 1.0.1 Record references to the tasks
TaskHandle_t cw_task_handle = NULL;
TaskHandle_t select_phrase_task_handle = NULL;
TaskHandle_t txtbuffer_task_handle = NULL;

    // These are the Send CW timers
volatile TimerHandle_t cwp_timer;
volatile TimerHandle_t cwd_timer;

    // These are the inter-task queues
volatile QueueHandle_t xQtimerp = NULL;
volatile QueueHandle_t xQtimerd = NULL;
volatile QueueHandle_t xQueue3 = NULL;
volatile QueueHandle_t xQbuffer = NULL;
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
    UBaseType_t uxMessagesWaiting  = 0;
    char this_char = 0;
    char last_char = 0;
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op tp_info = {1,1};
     
    // Selection of Texts
    char **pstrings;
    char *pnext_string;
    
    char pfox[] = "THE QUICK BROWN FOX JUMPED OVER THE LAZY DOGS BACK 0123456789\0";
    char pabc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\0";
    char pnumbers[] = "0123456789\0";
    char paris[] = "PARIS PARIS PARIS PARIS PARIS\0";
    char pve[]  = "VE7ZYX\0";
    char pvel[] = "ve7ZYX\0";
    char pcq[] = "CQ CQ CQ DE VE7ZYX CQ CQ CQ DE VE7ZYX K\0";
    char pph[RING_SIZE];
                                
    char *phrases[9] = {pfox,pabc,pnumbers,paris,pve,pvel,pcq,pph,0x0000};
    pstrings = phrases;  // transmit just one string pointed to by ptr
    
    vTaskDelay(ms_delay300);
    uint32_t first_flag  = 1;
    uint32_t timer_flag  = 0;
    


   while (true) {
        // first chcck to see if there is a new phrase waiting from Keyboard
         uxMessagesWaiting = uxQueueMessagesWaiting(xQbuffer);
         if(uxMessagesWaiting){
            for(i=0;i<RING_SIZE; i++) { // purge pph buffer
                pph[i] = '\0'; // fill buffer with string end char
            }
           xQueueReceive(xQbuffer,&pph, portMAX_DELAY);
         }
     if(timer_flag == 0) {  // Timer has triggered timeout
            xQueuePeek(xQphrase5, &phrase_select, portMAX_DELAY); 
            pnext_string = phrases[phrase_select];  // select string to be TX
            i = 0;  // sets up start of TX phrase
            last_char = 0x40;  // dummy value
            while ((pnext_string[i] != 0) && (i < (RING_SIZE-1))){   // step through TX phrase
                this_char = pnext_string[i]; 
                printf("%c",this_char);
                send_CW(this_char);
                i++;  // point to next character
                last_char = this_char;  // to correct "short" SPACE Char
  
            }  // end of while(next_string[i] ...
            printf("\n");
     
        } // end of if(timer_flag == 0) ...
        // Trigger one-shot timer to delay next CW text output
        if (cwp_timer != NULL) xTimerStart(cwp_timer, 0);
        xQueueReceive(xQtimerp, &tp_info, portMAX_DELAY); // xQueueReceive empties queue
        timer_flag = tp_info.t_state;
        tp_info.t_state = 1;

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
    struct op td_info = {1,0};

        gpio_put(DOTR, 1);  //  Dots need LOW to turn ON  
        gpio_put(DOTL, 1); // 

// *********************************************************************************************     
//                        A      B      C     D    E      F      G     H     I     J
    uint32_t morse[] = {0x11D,0x1157,0x45D7,0x457,0x11,0x1175,0x1177,0x455,0x45,0x11DDD,
//                        0      1      2     3    4      5      6     7     8     9
// *********************************************************************************************     
//        K      L     M     N      O      P      Q      R     S     T 
       0X11D7,0X115D,0X477,0X117,0X4777,0X22DD,0X11D77,0X45D,0X115,0X47,
//       10     11    12    13     14     15     16     17    18    19
// *********************************************************************************************     
//        U      V      W     X       Y       Z       0       1        2       3  
       0X475,0X11D5,0X11DD,0X4757,0X11DD7,0X4577,0X477777,0x11DDDD,0X47775,0X11DD5,
//       20     21     22    23      24      25      26      27       28      29  
// *********************************************************************************************     
//        4      5      6      7        8       9     SP  SPlg    .        , 
       0X4755,0X1155,0X4557,0X11577,0X45777,0X117777,0X10,0X80,0X11D75D,0X477577,
//       30     31     32     33       34      35     36   37     38      39 
// *********************************************************************************************     
//        ?       /        '
       0x45775,0X11757,0X15DDDD};
//        40      41       42
// *********************************************************************************************     
         
        this_char = ascii_in;
       
        if (this_char == 44) { // look for COMMA char
            this_char = 39;    // index to Morse char
            }
        else if (this_char == 46) { // look for PERIOD char
            this_char = 38;    // index to Morse char
            }
        else if (this_char == 63) { // look for QUESTION char
            this_char = 40;    // index to Morse char
            }
        else if (this_char == 47) { // look for FORWARD SLASH char
            this_char = 41;    // index to Morse char
            }
        else if (this_char == 39) { // look for APOSTROPHY char
            this_char = 42;    // index to Morse char
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
            this_char = 36; // SPACE long
            }      
                
        morse_out = morse[this_char];     // get Morse char from array
        
    while (morse_out != 1) {              // send Morse bits              
        bit_out = (morse_out & masknumber);  // isolate bit to send
        gpio_put(DOTR, !bit_out);  //  Dots need LOW to turn ON  
        gpio_put(DOTL, !bit_out); // 
        
        morse_out /= 2;                   // divide by 2 to shift Right
        if (cwd_timer != NULL) {
            xTimerStart(cwd_timer, 0);  // define dot length 
            xQueueReceive(xQtimerd, &td_info, portMAX_DELAY); // xQueueReceive empties queue
        }
    }

} 

/**
 * @brief Repeat check of txtctl, stop and start blinking
 * Waits for text input (YES/NO). If "YES" blink. IF "NO" stop blinking.
 * Sends command string to on_off_task with qbuffer mailbox
 * as a control signal.
 * BACKSPACE deletes characters entered to correct entry mistakes
 */
void txt_buffer_task(void* unused_arg) {
    uint8_t i = 0; //  counter
    uint8_t char_position = 0; //  counter
    uint32_t char_count = 0; //  counter
    char ring[RING_SIZE];
    UBaseType_t xStatus;
    char new_char = 'A';

   UBaseType_t uxNumberOfQSpaces = 1;
    
    while(true){
        for(i=0;i<RING_SIZE; i++) { 
            ring[i] = '\0'; // fill buffer with string end char
            }

          for(i=0;i<RING_SIZE; i++) { 
            new_char = get_iso8859_code();  // from ps2.c 
            if(new_char == 0) break;

            if(new_char == 0x0d)break;
            
            new_char = toupper(new_char);

            ring[i] = new_char; 
            vTaskDelay(ms_delay50);
            }
            if(new_char != 0){
                uxNumberOfQSpaces = uxQueueSpacesAvailable( xQbuffer );
                if(uxNumberOfQSpaces > 0){
                    xStatus = xQueueSendToFront(xQbuffer,&ring, 0);
                }
            }
    vTaskDelay(ms_delay237);
    }  // End while (true)    
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
 * @brief Callback actioned when the CW timer fires.  Sends trigger to
 * initiate a new CW String TX.
 *
 * @param timer: The triggering timer.
 */
void cwp_timer_fired_callback(TimerHandle_t timer) {
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op timer_info = {1,0};

    if (timer == cwp_timer) {
        // The timer fired so trigger cw ID in cw task
        xQueueOverwrite(xQtimerp, &timer_info);
       }
}


/**
 * @brief Callback actioned when the CW timer fires.  Sends trigger to
 * initiate a new CW String TX.
 *
 * @param timer: The triggering timer.
 */
void cwd_timer_fired_callback(TimerHandle_t timer) {
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    };  
    struct op timer_info = {2,0};

    if (timer == cwd_timer) {
        // The timer fired so trigger cw ID in cw task
        xQueueOverwrite(xQtimerd, &timer_info);
       }
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
 * @brief Set active VT100 screen window. 
 */
void windowT(void) {
    printf("\x1B[6;15r"); // set window lines 6;8
    printf("\x1B[%i;%iH**********************",6,2);  // place curser
    printf("\x1B[%i;%iH**********************",8,2);  // place curser
    printf("\x1B[%i;%iH\x1B[2K",7,3);  // place curser, clear rest of line
}

/**
 * @brief Set active VT100 screen window. 
 */
void windowB(void) {
    printf("\x1B[9;11r"); // set window lines 9;11
    printf("\x1B[%i;%iH**********************",9,2);  // place curser
    printf("\x1B[%i;%iH**********************",11,2);  // place curser
    printf("\x1B[%i;%iH\x1B[2K",10,3);  // place curser, clear rest of line
}


/**
 * @brief Initialize GPIO Pin for input and output.
 *
 */
void configure_gpio(void) {
    uint32_t pico_led_state = 0;

    // Enable STDIO
    stdio_init_all();

    // Initialize PS2 Keyboard
    kbd_init();
    
    // Initialize port extender
    pcf8575_init();
    
    stdio_usb_init();
    // Pause to allow the USB path to initialize
    sleep_ms(2000);
    
    // Configure PICO_LED_PIN for Initialization failure warning
    gpio_init(PICO_LED_PIN);
    gpio_disable_pulls(PICO_LED_PIN);  // remove pullup and pulldowns
    gpio_set_dir(PICO_LED_PIN, GPIO_OUT);

    // Configure DOTL LED_PIN 
    gpio_init(DOTL);
    gpio_disable_pulls(DOTL);  // remove pullup and pulldowns
    gpio_set_dir(DOTL, GPIO_OUT);

    // Configure DOTR_LED_PIN 
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
    char ring[RING_SIZE];
    struct op
    {
        uint32_t sw_number;
        uint32_t sw_state;
    }; 
    struct op sw_info = {0,0};

    configure_gpio();
    
    // label Program Screen
    printf("\x1B[2J");  // Clear Screen
    printf("\x1B[%i;%iH",2,3);  // place curser
    puts("*** CW Sending  Program ***");
    printf("\x1B[%i;%iH",4,3);  // place curser
    puts("**************************************");
    printf("\x1B[%i;%ir",6,15);  // set window top and bottom lines
 
// Timer creates pause between repetition of the CW Text
    cwp_timer = xTimerCreate("CWP_TIMER", 
                            PAUSE_PERIOD,
                            pdFALSE,
                            (void*)PHRASE_TIMER_ID,
                            cwp_timer_fired_callback);
        if (cwp_timer == NULL) {
            error_state  += 1;
            }

// Timer creates dot length
    cwd_timer = xTimerCreate("CWD_TIMER", 
                            DOT_PERIOD,
                            pdFALSE,
                            (void*)DIT_TIMER_ID,
                            cwd_timer_fired_callback);
        if (cwd_timer == NULL) {
            error_state  += 1;
            }
            

    // Set up tasks
    // FROM 1.0.1 Store handles referencing the tasks; get return values
    // NOTE Arg 3 is the stack depth -- in words, not bytes

    BaseType_t cw_status = xTaskCreate(cw_task, 
                                         "CW_LED_PIN_TASK", 
                                         256, 
                                         NULL, 
                                         6, 
                                         &cw_task_handle);
        if (cw_status != pdPASS) {
            error_state  += 1;
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

    BaseType_t select_phrase_task_status = xTaskCreate(select_phrase_task, 
                                         "SELECT_PHRASE_TASK", 
                                         256, 
                                         NULL, 
                                         5,     // Task priority
                                         &select_phrase_task_handle);
        if (select_phrase_task_status != pdPASS) {
            error_state  += 1;
            }
             
   // Set up the event queue
    xQtimerp = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQtimerp == NULL ) error_state += 1;

    xQtimerd = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQtimerd == NULL ) error_state += 1;

    xQueue3 = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQueue3 == NULL ) error_state += 1; 

    xQbuffer = xQueueCreate(1, sizeof(ring));
    if ( xQbuffer == NULL ) error_state += 1;
     
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
