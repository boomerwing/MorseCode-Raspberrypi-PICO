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
#define DECKSIZE 26

/* 
 * GLOBALS
 */
    // FROM 1.0.1 Record references to the tasks
TaskHandle_t cw_task_handle = NULL;
TaskHandle_t select_phrase_task_handle = NULL;
TaskHandle_t shuffle_task_handle = NULL;
TaskHandle_t sw1_task_handle = NULL;

    // These are the Send CW timers
volatile TimerHandle_t cwp_timer;
volatile TimerHandle_t cwd_timer;

    // These are the inter-task queues
volatile QueueHandle_t xQpause = NULL;
volatile QueueHandle_t xQdit = NULL;
volatile QueueHandle_t xQshuffle = NULL;
volatile QueueHandle_t xQsw1 = NULL;
volatile QueueHandle_t xQphrase5 = NULL;



/*
 * FUNCTIONS
 */
/*  
 * @brief Shuffle deck of size DECKSIZE, 
 *  */

 void shuffle_task(void* unused_arg) {
    UBaseType_t uxNumberOfQSpaces;
    UBaseType_t uxMessagesWaiting  = 0;
    UBaseType_t xStatus;
    uint8_t i = 0;   // useful counter
    uint8_t j = 0;   // 
    uint8_t deck_size = DECKSIZE;  // initialize deck_size variable
    uint8_t sw_state = 1;         // initialize sw1_state
    uint8_t pick_place = 0;        // initialize position of char to be picked
    uint8_t next_pos = 0;          // initialize next_pos in Result Deck
    uint8_t sw1_buffer = 0;  // initialize Queue input buffer
    char pick_char = 63;            // initialize first char picked
    char deck1[DECKSIZE + 1];       // deck 1 preshuffle array
    char deck2[DECKSIZE + 1];       // deck 2 post-shuffle arrray
    char label[DECKSIZE + 1];       // label arrray
    

//  create random seed with initial Toggle of Switch 3
        while(uxMessagesWaiting == 0){
            vTaskDelay(ms_delay50);  // Break for other tasks
            uxMessagesWaiting = uxQueueMessagesWaiting(xQsw1);
            if(uxMessagesWaiting){
            xQueueReceive(xQsw1,&sw1_buffer, portMAX_DELAY);
            sw_state = sw1_buffer;
            } 
        }
         if (sw_state == 1) {
//                printf("\x1B[%i;%iH",6,0);
//                printf("\x1B[K");  //  clear to end of Line Right
//                printf("  ** Set Switch1 UP ** "); 
            while (sw_state == 1){
                vTaskDelay(ms_delay50);  // Break for other tasks
                j += (rand() % 137 * rand() % 79);
                uxMessagesWaiting = uxQueueMessagesWaiting(xQsw1);
                if(uxMessagesWaiting){
                    xQueueReceive(xQsw1,&sw1_buffer, portMAX_DELAY); 
                    sw_state = sw1_buffer;
                }
            } 
        }
//            printf("\x1B[%i;%iH",6,0);  // place curser
//            printf("\x1B[K");  //  clear to end of Line Right
//            printf("  *** Set Switch1 Down then Up ***");
//            printf("\x1B[%i;%ir",10,20);  // set top and bottom lines of window
       while (sw_state == 1){
            vTaskDelay(ms_delay50);  // Break for other tasks
            j += (rand() % 137 * rand() % 79);
            uxMessagesWaiting = uxQueueMessagesWaiting(xQsw1);
            if(uxMessagesWaiting){
                xQueueReceive(xQsw1,&sw1_buffer, portMAX_DELAY); 
                sw_state = sw1_buffer;
            }
        }
        while (sw_state == 0){  // look for switch to go to UP to start shuffle
            vTaskDelay(ms_delay50);  // Break for other tasks
            j += (rand() % 137 * rand() % 79);
            uxMessagesWaiting = uxQueueMessagesWaiting(xQsw1);
            if(uxMessagesWaiting){
            xQueueReceive(xQsw1,&sw1_buffer, portMAX_DELAY); 
            sw_state = sw1_buffer;
                }
        }
     srand48(j);
    
        // label both orig and final decks
    for (i = 0; i < DECKSIZE; i++) {
        j = (i %10) +1;
        if (j == 10) j = 0;
            label[i] = 48 +j;
        }
//    label[DECKSIZE]  = '\0';  //  add end to the Label string
//    printf("\x1B[%i;%iH",8,0);  //  place curser
//    printf("\x1B[K");  //  clear to end of Line Right
//    printf("%s",label);    //  print out label string
                                 
    // initialize orig deck with Capital Alphabetic Characters
    for (i = 0; i < DECKSIZE; i++) {
        deck1[i] = i + 65; 
        }
        deck1[deck_size] = '\0';  //  add end to the unshuffled deck string

    for (i = 0; i < DECKSIZE; i++) {  // initialize final deck with '*' string
        deck2[i] = '*';
        }
        deck2[DECKSIZE]  = '\0';  //  add end to the shuffled deck string

// Beginning of each Shuffle
    while (true) {
        sw_state = 1;
       while (sw_state == 1){
         uxMessagesWaiting = uxQueueMessagesWaiting(xQsw1);
         vTaskDelay(ms_delay50);  // Break for other tasks
          if(uxMessagesWaiting){
           xQueueReceive(xQsw1,&sw1_buffer, portMAX_DELAY); 
            sw_state = sw1_buffer;
            }
       }
        while (sw_state == 0){  // look for switch to go to UP to start shuffle
         uxMessagesWaiting = uxQueueMessagesWaiting(xQsw1);
         vTaskDelay(ms_delay50);  // Break for other tasks
          if(uxMessagesWaiting){
           xQueueReceive(xQsw1,&sw1_buffer, portMAX_DELAY); 
            sw_state = sw1_buffer;
            }
        }
        sw_state = 1;

        deck_size = DECKSIZE;  //  reset start position of original deck
        next_pos = 0;          // ** reset start position of final deck **

            /* deck_size shortens as each random character is removed and the 
             * hole closed. Deck1 shortens as Deck2 is Filled. 
             */
        while(deck_size > 0) {
            pick_place = rand() % deck_size;
                                                 //  end of the before-after string
            pick_char = deck1[pick_place];
            deck2[next_pos] = pick_char;
            next_pos += 1;  // point to next Deck2 insert position
        
            for (i = pick_place; i < deck_size; i++) { // close source deck after pick
                 deck1[i] = deck1[i+1];
                }
            deck_size = deck_size - 1;             // shorten deck_size after pick and close of orig deck
            deck1[deck_size] = '\0';  //  add end to the original string
            deck2[DECKSIZE]  = '\0';  //  add end to the resulting string

            vTaskDelay(ms_delay10);  // Break for other tasks
            }  // end of while(deck_size > 0)
      // setup for next shuffle 
        for (i = 0; i < DECKSIZE; i++) {
            deck1[i] = deck2[i];
            deck2[i] = '*';  
            }
        if(uxQueueSpacesAvailable( xQshuffle )){
            xStatus = xQueueSendToFront(xQshuffle,&deck1, portMAX_DELAY);
        }

        vTaskDelay(ms_delay300);  // Break for other tasks
      
   } //   while(true);
}



/**
 * @brief Send CW, blinking RX LED 
 *        xQtimer1 triggers TX of phrase chosen by ADC input value
 */
void cw_task(void* unused_arg) {
    UBaseType_t uxMessagesWaiting  = 0;
    UBaseType_t xStatus;

    uint32_t i  = 1;
    uint32_t number  = 1;
    uint32_t phrase_select  = 1;
    uint32_t phrase_length  = 0;
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
    char pve[]  = "VE7BGV\0";
    char pvel[] = "ve7bgv\0";
    char pcq[] = "CQ CQ CQ DE VE7BGV CQ CQ CQ DE VE7BGV K\0";
    char pph[64];
                                
    char *phrases[9] = {pfox,pabc,pnumbers,paris,pve,pvel,pcq,pph,0x0000};
    pstrings = phrases;  // transmit just one string pointed to by ptr
    
    vTaskDelay(ms_delay300);
    uint32_t first_flag  = 1;
    uint32_t timer_flag  = 0;
    


   while (true) {
        // first check to see if there is a new phrase waiting from shuffle task
          if(uxQueueMessagesWaiting(xQshuffle)){
            for(i=0;i<64; i++) { // purge pph buffer
                pph[i] = '\0'; // fill buffer with string end char
            }
           xQueueReceive(xQshuffle,&pph, portMAX_DELAY);
         }
         
        // check to see if there is a new phrase selection waiting from switches         
         uxMessagesWaiting = uxQueueMessagesWaiting(xQphrase5);
         if(uxMessagesWaiting){
            xQueueReceive(xQphrase5,&phrase_select, portMAX_DELAY); 
            }
            
        pnext_string = phrases[phrase_select];  // select string to be TX
        i = 0;  // sets up start of TX phrase
        last_char = 0x40;  // dummy value
        while ((pnext_string[i] != 0) && (i < (64))){   // step through TX phrase
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
            i++;  // point to next character
            last_char = this_char;  // to correct "short" SPACE Char
        }  // end of while(next_string[i] ...
        printf("\n");
     
        // Trigger one-shot timer to delay next CW text output
        if (cwp_timer != NULL){ 
            xTimerStart(cwp_timer, 0);
            }
             xStatus = xQueueReceive(xQpause, &tp_info, portMAX_DELAY); // xQueueReceive empties queue
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
    UBaseType_t xStatus;
    UBaseType_t uxNumberOfQSpaces = 1;
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op tdit_info = {1,1};

    
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
        if (cwd_timer != NULL) {
            xTimerStart(cwd_timer, 0);  // define dot length 
            }
             xStatus = xQueueReceive(xQdit, &tdit_info, portMAX_DELAY); // xQueueReceive empties queue
       }

} 


/**
 * @brief Callback actioned when the CW timer fires.  Sends trigger to
 * initiate a new CW String TX.
 *
 * @param timer: The triggering timer.
 */
void cwp_timer_fired_callback(TimerHandle_t timer) {
    UBaseType_t xStatus;
    UBaseType_t uxNumberOfQSpaces = 1;
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op timer_info = {1,0};

    if (timer == cwp_timer) 
        // The timer fired so trigger cw ID in cw task
        xQueueOverwrite(xQpause, &timer_info);
}



/**
 * @brief Callback actioned when the CW timer fires.  Sends trigger to
 * initiate a new CW String TX.
 *
 * @param timer: The triggering timer.
 */
void cwd_timer_fired_callback(TimerHandle_t timer) {
    UBaseType_t xStatus;
    UBaseType_t uxNumberOfQSpaces = 1;
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op timer_info = {2,0};

    if (timer == cwd_timer)  // define dot length 
         // The timer fired so trigger cw ID in cw task
        xQueueOverwrite(xQdit, &timer_info);
}

/* 
 * @brief Read SW1, SW2 and SW3, calculate binary value, 
 * as a control signal, passed to an action function by a Queue. 
 * ADC reading divided into segments. Value of segments displayed on
 * a Seven Segment LED module
 */

 void select_phrase_task(void* unused_arg) {  // selects which Text phrase to o/p
    UBaseType_t xStatus;
    UBaseType_t uxNumberOfQSpaces = 1;

    const uint8_t  sw_input_mask = 0b00000111;
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
    
        uxNumberOfQSpaces = uxQueueSpacesAvailable( xQphrase5 );
        if(uxNumberOfQSpaces > 0){
            xStatus = xQueueSendToFront(xQphrase5,&select_val, 0);
        }
      vTaskDelay(ms_delay300);  // check adc value every 300 ms
    }  // End while (true)    
}

/**
 * @brief Switch Debounce 
 * Repeat check of SW, send result to miso led pin task to stop and start blinking
 * Measures sw state, compares NOW state with PREVIOUS state. If states are different
 * sets count == 0 and looks for two states the same.  It then looks for five or more (MIN_COUNT)
 * in a row or more where NOW and PREVIOUS states are the same. Then Switch state is used
 * as a control signal, passed to an action function by a Queue.
 */
 void sw1_debounce(void* unused_arg) {
    uint8_t now = 1;            // initialize sw1_state
    uint8_t last = 1;   // initialize sw1_previous_state
    uint32_t count = 1;   // initialize sw1_final_state
    uint8_t pcfbuffer[2] = {0b11111111,0b11111111};
    UBaseType_t xStatus;
    UBaseType_t uxNumberOfQSpaces = 1;
    
    while (true) {
        // Measure SW and add the LED state
        // to the FreeRTOS xQUEUE
        last = now;
        i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, true);
        now = readBit(pcfbuffer[0], P03);
        if(last == now) {
            vTaskDelay(ms_delay50);  // check switch state every 50 ms
        }
        else{   //  if previous state |= state switch has changed
            count = 0;
            while(count < 3){
                last = now;
                i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, true);
                now = readBit(pcfbuffer[0], P03);
                if(last == now) { // switch has stopped bouncing
                    count++;
                }
                else{ // switch is still bouncing
                    count = 0;
                }
            vTaskDelay(ms_delay5);  // check switch state every 5 ms
            }
            uxNumberOfQSpaces = uxQueueSpacesAvailable( xQsw1 );
            if(uxNumberOfQSpaces > 0){
                xStatus = xQueueSendToFront(xQsw1,&now, 0);
            }
        }
    }  // End while (true)    
}


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
    char shufflestr[64];
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
            
     BaseType_t shuffle_status = xTaskCreate(shuffle_task, 
                                         "SHUFFLE_TASK", 
                                         256, 
                                         NULL, 
                                         5,     // Task priority
                                         &shuffle_task_handle);
        if (shuffle_status != pdPASS) {
            error_state  += 1;
            }
   
    BaseType_t select_phrase_task_status = xTaskCreate(select_phrase_task, 
             
                                         "SELECT_PHRASE_TASK", 
                                         256, 
                                         NULL, 
                                         4,     // Task priority
                                         &select_phrase_task_handle);
        if (select_phrase_task_status != pdPASS) {
            error_state  += 1;
            }
             
    BaseType_t sw1_status = xTaskCreate(sw1_debounce, 
                                         "SW1_TASK", 
                                         256, 
                                         NULL, 
                                         7,     // Task priority
                                         &sw1_task_handle);
        if (sw1_status != pdPASS) {
           error_state  += 1;
            }
    
   // Set up the event queue
    xQpause = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQpause == NULL ) error_state += 1;

    xQdit = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQdit == NULL ) error_state += 1;

    xQsw1 = xQueueCreate(1, sizeof(uint8_t)); 
    if ( xQsw1 == NULL ) error_state += 1; 

    xQshuffle = xQueueCreate(1, sizeof(shufflestr));
    if ( xQshuffle == NULL ) error_state += 1;
     
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
