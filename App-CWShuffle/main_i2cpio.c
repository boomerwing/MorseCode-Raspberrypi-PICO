/**
 * RP2040 FreeRTOS Template: Send CW with Timer trigger
 * Timer is monostable, creating delay between text sent
 * Also learning to use structures in Queue
 * Sending Morse Code text
 * pio function provides a 600 Hz Square wave
 * pio output and CW output are two inputs to a Common Collector NOR Gate
 * A CW LOW blocks the PIO Square wave output
 * ADC reads in a Potentiometer setting to select one of six installed phrases
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
// ***************** PIO Includes *****************
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "blink.pio.h"
// ************************************************

#define PAUSE_PERIOD 2800
#define DOT_PERIOD 67
#define I2C_ADDR 0x20
#define SPTOOSIZE 74

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


void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq);

/*
 * FUNCTIONS
 */

/*
 * blink_pin_forever - generate a square wave out of gpio pin
 */

void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

//    printf("Blinking pin %d at %d Hz\n", pin, freq);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (clock_get_hz(clk_sys) / (2 * freq)) - 3;
}

/*  
 * @brief Shuffle deck of size DECKSIZE, 
 *  */

 void shuffle_task(void* unused_arg) {
    UBaseType_t uxNumberOfQSpaces;
    UBaseType_t uxMessagesWaiting  = 0;
    UBaseType_t xStatus;
    uint8_t i = 0;   // useful counter
    uint8_t j = 0;   // 
    uint8_t now = 4;   // 
    uint8_t last= 4;   // 
    uint8_t deck_size = SPTOOSIZE;  // initialize deck_size variable
    uint8_t sw_state = 1;          // initialize sw1_state
    uint8_t pick_place = 0;        // initialize position of char to be picked
    uint8_t next_pos = 0;          // initialize next_pos in Result Deck
    uint8_t sw1_buffer = 1;        // initialize Queue input buffer
    uint8_t phrase_select  = 1;
    uint8_t phrase_length  = 0;
    uint8_t deck_sizeorg;  //  reset start position of original deck
    char pick_char = 63;           // initialize first char picked
    char deck1[SPTOOSIZE];      // deck 1 preshuffle array
    char deck2[SPTOOSIZE];      // deck 2 post-shuffle arrray
    char sptoo[SPTOOSIZE];         // deck with spaces
    
    // Selection of Texts
    char **pstrings;
    char *pnext_string;
    
    char pfox[] = "THE QUICK BROWN FOX JUMPED OVER THE LAZY DOGS BACK 0123456789\0";
    char pabc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZVE7BGVABCDEFGHIJKLMNOPQRSTUVWXYZ\0";
    char pnumbers[] = "01234567890123456789012345678901234567890123456789678912\0";
    char pve[]  = "AB1CD2EF3GH4IJ5KL6MN7OP8QR9ST0UVWXYZVE7BGV\0";
    char psha[] = "XAMTGIBSDZUXPINECFHEFSJTELVAYRAGCYWVFOJBPCUBDQNGRKZOMKWDHQL\0";
    char pshb[] = "XAMTGIBSDZUXPINECFHEFSJTELVAYRAGCYWVFOJBPCUBDQNGRKZOMKWDHQL0123456789\0";
    char pshc[] = "N6L05XIQVCV8WRG4TEDSFBK1EPBMUO7J79A2G3YVZH\0";
    char pph[SPTOOSIZE];
                                
    char *phrases[9] = {pfox,pabc,pnumbers,pve,psha,pshb,pshc,pph,0x0000};
    pstrings = phrases;  // transmit just one string pointed to by ptr

//  create random seed with initial Toggle of Switch 3
//      first wait for sw1 to be pressed and released.
        uxMessagesWaiting = uxQueueMessagesWaiting(xQsw1);
        while(uxMessagesWaiting == 0){
            vTaskDelay(ms_delay50);  // Break for other tasks, while waiting for first button press
            uxMessagesWaiting = uxQueueMessagesWaiting(xQsw1);
            if(uxMessagesWaiting){
                xQueueReceive(xQsw1,&sw1_buffer, portMAX_DELAY);
                sw_state = sw1_buffer;
                gpio_put(DOTL, !sw_state); // Visual indication of switch action
            } 
        }
//      sw1 pressed makes sw_state == 0, which calculsates one random no.
         if (sw_state == 0) {
            while (sw_state == 0){
                vTaskDelay(ms_delay50);  // Break for other tasks
                j += (rand() % 137 * rand() % 79);
                uxMessagesWaiting = uxQueueMessagesWaiting(xQsw1);
                if(uxMessagesWaiting){
                    xQueueReceive(xQsw1,&sw1_buffer, portMAX_DELAY); 
                    sw_state = sw1_buffer;     // change sw_state to 0
                    gpio_put(DOTL, !sw_state); //  Visual indication of switch action
   
                }
            } 
        }
//      sw1 released makes sw_state == 1, which calculsates another random no.
        while (sw_state == 1){
            vTaskDelay(ms_delay50);  // Break for other tasks
            j += (rand() % 137 * rand() % 79);
            uxMessagesWaiting = uxQueueMessagesWaiting(xQsw1);
            if(uxMessagesWaiting){
                xQueueReceive(xQsw1,&sw1_buffer, 0); 
                sw_state = sw1_buffer; // change sw_state to 1
                 gpio_put(DOTL, !sw_state);  // Visual indication of switch action 
          }
        }
//      sw1 pressed makes sw_state == 0, which calculsates one random no.
        while (sw_state == 0){  // look for switch to go to UP to start shuffle
            vTaskDelay(ms_delay50);  // Break for other tasks
            j += (rand() % 137 * rand() % 79);
            uxMessagesWaiting = uxQueueMessagesWaiting(xQsw1);
            if(uxMessagesWaiting){
                xQueueReceive(xQsw1,&sw1_buffer, 0); 
                sw_state = sw1_buffer; // change sw_state to 0
                gpio_put(DOTL, !sw_state); //  Visual indication of switch action
                }
        }
//      sw1 released makes sw_state == 1, which calculsates another random no.
        while (sw_state == 0){
            vTaskDelay(ms_delay50);  // Break for other tasks
            j += (rand() % 137 * rand() % 79);
            uxMessagesWaiting = uxQueueMessagesWaiting(xQsw1);
            if(uxMessagesWaiting){
                xQueueReceive(xQsw1,&sw1_buffer, 0); 
                sw_state = sw1_buffer;  // change sw_state to 1 to begin shuffle
                gpio_put(DOTL, !sw_state); //  Visual indication of switch action
           }
        }
     srand48(j);  // random number to start shuffle

    for (i = 0; i < SPTOOSIZE; i++) {  // initialize deck1 with '\0'
        deck1[i] = '\0';
        }
        
    for (i = 0; i < SPTOOSIZE; i++) {  // initialize deck2 with '\0' string
        deck2[i] = '\0';
        }

// Beginning of each Shuffle
    while (true) {
        // check to see if there is a new phrase selection waiting from switches         
         uxMessagesWaiting = uxQueueMessagesWaiting(xQphrase5);
         if(uxMessagesWaiting){
            xQueueReceive(xQphrase5,&phrase_select, 0); 
            pnext_string = phrases[phrase_select];  // select string to be TX
            for(i=0; i<SPTOOSIZE;i++){
                deck1[i] = '\0';
                deck2[i] = '\0';
                }
            for(i=0; i<(strlen(pnext_string));i++){
                deck1[i] = pnext_string[i];
                }
            }

        if(phrase_select < 4) {
            if(uxQueueSpacesAvailable( xQshuffle )){
                xStatus = xQueueSendToFront(xQshuffle,&deck1,portMAX_DELAY);
           }
        }
        else {
            deck_size = strlen(deck1);  //  reset start position of original deck
            deck_sizeorg = deck_size;   //  reset start position of original deck
            next_pos = 0;               // ** reset start position of final deck **

                /* deck_size shortens as each random character is removed and the 
                 * hole closed. Deck1 shortens as Deck2 is Filled. 
                 */
            while(deck_size > 0) {
                pick_place = rand() % deck_size;
                                                     //  end of the before-after string
                pick_char = deck1[pick_place];
                deck2[next_pos] = pick_char;
                next_pos += 1;  // Deck2 grows.  Point to next Deck2 insert position
            
                for (i = pick_place; i < deck_size; i++) { // close source deck after pick
                     deck1[i] = deck1[i+1];
                    }
                deck_size = deck_size - 1;  // shorten deck_size after "pick and close" of orig deck
                deck1[deck_size] = '\0';    // add end to the original string

                vTaskDelay(ms_delay10);     // Break for other tasks
                }  // end of while(deck_size > 0)
                
        // Now write deck2 into sptoo adding spaces after every five letters

            // purge sptoo deck with '\0'
            for (i = 0; i < SPTOOSIZE; i++) {
                sptoo[i] = '\0'; 
                }
                
            j = 0; 
            for(i=0;i<(strlen(deck2));i++,j++){
                if(i == 0){
                    sptoo[j] = deck2[i];
                }
                else if((i%5) == 0){
                    sptoo[j++] = 0x20;  // after every 5 letters, add a space to output
                    sptoo[j] = deck2[i];
                }
                else {
                    sptoo[j] = deck2[i];
                }
            }
            
        // then send spt00 string to string pph of CW task

            if(uxQueueSpacesAvailable( xQshuffle )){
                xStatus = xQueueSendToFront(xQshuffle,&sptoo,portMAX_DELAY);
           }
          // setup for next shuffle, move deck2 to deck1, empty deck2  
            for (i = 0; i < deck_sizeorg; i++) {
                deck1[i] = deck2[i];
                deck2[i] = '\0';  
                }
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

    uint8_t i  = 1;
    uint32_t number  = 1;
    uint8_t phrase_select  = 1;
    char this_char = 0;
    char last_char = 0;
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op tp_info = {1,1};
     
    char pph[SPTOOSIZE];
    char cmdstring[] = "Press and Release Switch two times\0";


    // Selection of Texts
    char **pstrings;
    char *pnext_string; 
    
    vTaskDelay(ms_delay300);
    uint32_t first_flag  = 1;
    uint32_t timer_flag  = 0;
    
    pnext_string = cmdstring;

   while (true) {
        // first check to see if there is a new phrase waiting from shuffle task
          if(uxQueueMessagesWaiting(xQshuffle)){
            for(i=0;i<SPTOOSIZE; i++) { // purge pph buffer
                pph[i] = '\0'; // fill buffer with string end char
            }
           xStatus = xQueueReceive(xQshuffle,&pph, 0);
            pnext_string = pph;  // select string to be TX
         }
        
        i = 0;  // sets up start of TX phrase
        last_char = 0x40;  // dummy value
        while ((pnext_string[i] != '\0') && (i < SPTOOSIZE)){   // step through TX phrase
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
            last_char = this_char; 
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
        gpio_put(DOTR, !bit_out);         //  Dots need LOW to turn ON  
        gpio_put(D13_PIN, bit_out);       //  Output to Audio Gate
        
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
    uint8_t select_val;
    uint8_t now;
    uint8_t last;
    uint8_t temp_sw_input;
    uint8_t  pcfbuffer[2]={0b11111111,0b11111111};// data buffer, must be two bytes
   
    while (true) {
        // Read Switches on i2c ports 0, 1, and 2 and add the LED state
        // to the FreeRTOS xQUEUE
      i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, false);
      temp_sw_input = pcfbuffer[0]; // protect input values from change
      select_val = (temp_sw_input &= sw_input_mask);
      show_seven_seg_i2c(select_val);
      last = now;
      now = select_val;
      if(now != last){
            uxNumberOfQSpaces = uxQueueSpacesAvailable( xQphrase5 );
            if(uxNumberOfQSpaces > 0){
                xStatus = xQueueSendToFront(xQphrase5,&select_val, 0);
            }
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
    uint8_t count = 4;   // initialize sw1_final_state
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
        else{   //  if last |= now switch has changed
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
            uxNumberOfQSpaces = uxQueueSpacesAvailable(xQsw1);
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

    // Configure DOTL_PIN
    gpio_init(DOTL);
    gpio_disable_pulls(DOTL);  // remove pullup and pulldowns
    gpio_set_dir(DOTL, GPIO_OUT);

    // Configure DOTR pin 
    gpio_init(DOTR);
    gpio_disable_pulls(DOTR);  // remove pullup and pulldowns
    gpio_set_dir(DOTR, GPIO_OUT);

    // Configure D12_PIN for square wave out of pio
    gpio_init(D12_PIN);
    gpio_disable_pulls(D12_PIN);  // remove pullup and pulldowns
    gpio_set_dir(D12_PIN, GPIO_OUT);

    // Configure D13_PIN for CW out to audio gate
    gpio_init(D13_PIN);
    gpio_disable_pulls(D13_PIN);  // remove pullup and pulldowns
    gpio_set_dir(D13_PIN, GPIO_OUT);

     // Configure GPIO Extender
    pcf8575_init();
    
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
    char pph[SPTOOSIZE];
    struct op
    {
        uint32_t sw_number;
        uint32_t sw_state;
    }; 
    struct op sw_info = {0,0};

    configure_gpio();
    
    // todo get free sm
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &blink_program);

    blink_pin_forever(pio, 0, offset, 12, 600);  // approx 600 Hz
//    blink_pin_forever(pio, 0, offset, 18, 3);  // gpio 18
//    blink_pin_forever(pio, 1, offset, 13, 4);  // gpio 13
//    blink_pin_forever(pio, 2, offset, 14, 1);  // gpio 14
    
    
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

    xQshuffle = xQueueCreate(1, sizeof(pph));
    if ( xQshuffle == NULL ) error_state += 1;
     
    xQphrase5 = xQueueCreate(1, sizeof(uint8_t));
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
