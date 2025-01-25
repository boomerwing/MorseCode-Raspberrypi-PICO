/**
 * RP2040 FreeRTOS Template: Send CW with Timer trigger
 * Timer is monostable, creating delay between text sent
 * Also learning to use structures in Queue
 * Sending Morse Code text
 * Text selected by value read in by ADC
 * cd ~/FreeRTOS-Play/build/App-CWShuffle
 * 
 * built on 
 * @copyright 2022, Tony Smith (@smittytone)
 * @version   1.2.0
 * @licence   MIT
 */
 
#include <stdio.h>
#include "main.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "../Common/Seven_Seg_i2c/seven_seg.h"
#include "../Common/PCF8575-i2c/pcf8575i2c.h"
/* ***************** PIO STUFF ****************************** */
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "blink.pio.h"
/* ********************************************************** */

#define INTERVALS 7
#define PAUSE_PERIOD 1800
#define DOT_PERIOD 65  // 65 is about 20 WPM
#define DECKSIZE 64
#define SPTOOSIZE 130
#define MIN_COUNT 4
#define MAX_COUNT 10
#define SW_MASK 0B11111111

/* 
 * GLOBALS
 */
    // FROM 1.0.1 Record references to the tasks
TaskHandle_t cw_task_handle = NULL;
TaskHandle_t shuffle_task_handle = NULL;
TaskHandle_t sw_task_handle = NULL;
TaskHandle_t adc_task_handle = NULL;
// TaskHandle_t select_phrase_task_handle = NULL;

    // These are the Send CW timers
volatile TimerHandle_t cwp_timer;
volatile TimerHandle_t cwd_timer; 

    // These are the inter-task queues
volatile QueueHandle_t xQpause = NULL;
volatile QueueHandle_t xQdit = NULL;
volatile QueueHandle_t xQshuffle = NULL;
volatile QueueHandle_t xQbin3 = NULL;
volatile QueueHandle_t xQsw7 = NULL;
volatile QueueHandle_t xQsw6 = NULL;
volatile QueueHandle_t xQsw5 = NULL;
volatile QueueHandle_t xQsw4 = NULL;
volatile QueueHandle_t xQsw3 = NULL;
volatile QueueHandle_t xQsw2 = NULL;
volatile QueueHandle_t xQsw1 = NULL;
volatile QueueHandle_t xQsw0 = NULL;

/*
 * FUNCTIONS
 */
void switch0(uint8_t);
void switch1(uint8_t);
void switch2(uint8_t);
void switch3(uint8_t);
void switch4(uint8_t);
void switch5(uint8_t);
void switch6(uint8_t);
void switch7(uint8_t);
void switchBin3(uint8_t);
 
void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq); 

void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

//    printf("Blinking pin %d at %d Hz\n", pin, freq);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (clock_get_hz(clk_sys) / (2 * freq)) - 3;
}

/*
 * FUNCTIONS
 */
 
/*  
 * @brief Shuffle deck of size DECKSIZE, 
 *  Switch 6 starts the shuffle action. Switches 2,3,4 are used to select which phrase is TX.
 *  */

 void shuffle_task(void* unused_arg) {
    UBaseType_t uxNumberOfQSpaces;
    UBaseType_t uxMessagesWaiting  = 1;
    UBaseType_t xStatus;
    uint8_t stringlength = 10;
    uint8_t i = 0;   // useful counter
    uint8_t j = 0;   // 
    uint8_t now = 4;   // 
    uint8_t last= 4;   // 
    uint8_t deck_sizeorg;           //  reset start position of original deck
    uint8_t deck_size;              // initialize deck_size variable
    uint8_t sw_state = 1;           // initialize sw1_state
    uint8_t pick_place = 0;         // initialize position of char to be picked
    uint8_t next_pos = 0;           // initialize next_pos in Result Deck
    uint8_t sw6_buffer = 1;         // initialize Queue input buffer
    uint32_t phrase_select  = 3;
    char pick_char = 63;            // initialize first char picked
    char deck1[SPTOOSIZE];          // deck 1 preshuffle array
    char deck2[SPTOOSIZE];          // deck 2 post-shuffle arrray
    char sptoo[SPTOOSIZE];          // deck with spaces
    
    // Selection of Texts
    char **pstrings;
    char *pnext_string;
/**    
    char p00[] = "OESTA EOSAT STOSE SAOTE TSE00 ASTES TSAEA ETAOS SOAET OASET TAAST SEOAE STSOA ESATT ASETA OSTAS SASTA TSTOS OEASE SOSEA\0";
    char p01[] = "MEU4S 5I0AT UT5I4 HMSAV 0USV5 4mHAI 0TVIM ATU5I V5SHU 4T0HM TOV54 HMUSA U0H4I EVHOT MVU4I VTSOE ITAVM MSVOE AEO5S TUSV4\0";
    char p02[] = "SINRO NSELT HSARL NTHIO NEOAR SOLAL ITENR OALNS LSAHR ENSTE HSOLA IHORE AHSSI SELRA LTENN ELSAN TELER HOINA ELIOH SIRNS\0";
    char p03[] = "ON NO THE SON HILL SAIL REAR ROLL NONE REST HERE RATE LANE NEAR RENT NEST STILL TREAT STONE NORTH LITTLE THRILL HOSTESS\0";
    char p04[] = "OUT CUT CAST GULF HULL COST RUSH CROSS FORCE LATIN TRUST GRILL STUFF FROST CLING FRESH SOUTH AFRICA SAIGON HIGGINS AUSTRIA\0";
    char p05[] = "FHZX16RI0NMP96C873CRJOAOTYQW2KEJLYQUVTBKU8XG79SWIFMDNBLZ5SAEFHZX16RI0NMP96C873CRJOAOTYQW2KEJLYQUVTBKU8XG79SWIFMDNBLZ5SAE\0";
    char p06[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ01234567896789ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ01234567896789\0";
    char pph[SPTOOSIZE];
    char *phrases[9] = {p00,p01,p02,p03,p04,p05,p06,pph,0x0000}; 
*/

    char pfox[] = "THE QUICK BROWN FOX JUMPED OVER THE LAZY DOGS BACK 0123456789\0";
    char pabc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ01234567896789\0";
    char pnumbers[] = "01234567890123456789012345678901234567890123456789678912\0";
    char paris[] = "PARIS PARIS PARIS PARIS\0";
    char pfour[] = "EQ6ZNKJF2YU3DVQMKNMRO9S78CSBXAXWTOC9FRGLWJ8IYAZP0EI61HTB7LU5\0";
    char pfive[]  = "FHZX16RI0NMP96C873CRJOAOTYQW2KEJLYQUVTBKU8XG79SWIFMDNBLZ5SAE\0";
    char psix[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ01234567896789\0";
    char pph[SPTOOSIZE];
    char *phrases[9] = {pfox,pabc,pnumbers,paris,pfour,pfive,psix,pph,0x0000};

    pstrings = phrases;  // transmit just one string pointed to by ptr

//  create random seed with initial Toggle of Switch 6
//      first wait for sw6 to be pressed and released.
        uxMessagesWaiting = uxQueueMessagesWaiting(xQsw6);
        while(!uxQueueMessagesWaiting(xQsw6)){
            vTaskDelay(ms_delay50);  // Break for other tasks, while waiting for first button press
            j += (rand() % 137 * rand() % 79);
            }
            if(uxQueueMessagesWaiting(xQsw6)){
                xQueueReceive(xQsw6,&sw6_buffer, portMAX_DELAY);
                sw_state = sw6_buffer;
                if(sw_state) gpio_put(DOTR, 0);
                else gpio_put(DOTR,1);
            } 
//      sw6 pressed makes sw_state == 0, which calculsates one random no.
    
        while(!uxQueueMessagesWaiting(xQsw6)){
                vTaskDelay(ms_delay50);  // Break for other tasks
                j += (rand() % 137 * rand() % 79);
            }
                if(uxQueueMessagesWaiting(xQsw6)){
                    xQueueReceive(xQsw6,&sw6_buffer, portMAX_DELAY); 
                    sw_state = sw6_buffer;     // change sw_state to 1
                if(sw_state) gpio_put(DOTR, 0);
                else gpio_put(DOTR,1);
                }
//      sw6 released makes sw_state == 1, which calculsates another random no.

        uxMessagesWaiting = uxQueueMessagesWaiting(xQsw6);
        while(!uxQueueMessagesWaiting(xQsw6)){
            vTaskDelay(ms_delay50);  // Break for other tasks, while waiting for second button press
            j += (rand() % 137 * rand() % 79);
            }
            if(uxQueueMessagesWaiting(xQsw6)){
                xQueueReceive(xQsw6,&sw6_buffer, portMAX_DELAY);
                sw_state = sw6_buffer;
                if(sw_state) gpio_put(DOTR, 0);
                else gpio_put(DOTR,1);
            } 
//      sw6 pressed makes sw_state == 0, which calculsates one random no.
    
        while(!uxQueueMessagesWaiting(xQsw6)){
                vTaskDelay(ms_delay50);  // Break for other tasks
                j += (rand() % 137 * rand() % 79);
            }
                if(uxQueueMessagesWaiting(xQsw6)){
                    xQueueReceive(xQsw6,&sw6_buffer, portMAX_DELAY); 
                    sw_state = sw6_buffer;     // change sw_state to 1
                if(sw_state) gpio_put(DOTR, 0);
                else gpio_put(DOTR,1);
                }
//      sw6 released makes sw_state == 1, which calculates another random no.

        
     srand48(j);  // random number to start shuffle

    for (i = 0; i < SPTOOSIZE; i++) {  // initialize deck1 with '\0'
        deck1[i] = '\0';
        }
        
    for (i = 0; i < SPTOOSIZE; i++) {  // initialize deck2 with '\0' string
        deck2[i] = '\0';
        }
// Get next String
    while (true) {
        // check to see if there is a new phrase selection waiting from switches         
         if(uxQueueMessagesWaiting(xQbin3)){
           xQueueReceive(xQbin3,&phrase_select, 0); 
            pnext_string = phrases[phrase_select];  // select string to be TX
            for(i=0; i<SPTOOSIZE;i++){
                deck1[i] = '\0';
                deck2[i] = '\0';
                }
            stringlength = strlen(pnext_string);
            for(i=0; i<stringlength;i++){
                deck1[i] = pnext_string[i];
                }
           }
    // then send spt00 string to string pph of CW task

        if(uxQueueSpacesAvailable( xQshuffle )){
            xStatus = xQueueSendToFront(xQshuffle,&deck1,portMAX_DELAY);
       }
      // setup for next shuffle, move deck2 to deck1, empty deck2  
        for (i = 0; i < deck_sizeorg; i++) {
            deck1[i] = deck2[i];
            deck2[i] = '\0';  
            }

        vTaskDelay(ms_delay300);  // Break for other tasks
      
   } //   while(true);
}




/**
 * @brief Send CW, blinking LED and sending signal to gate sounding tone
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
    char startcmd[] = "PRESS BUTTON TWICE TO START\0";


    // Selection of Texts
    char **pstrings;
    char *pnext_string;
    
    vTaskDelay(ms_delay300);
    uint32_t first_flag  = 1;
    uint32_t timer_flag  = 0;
    
    pnext_string = startcmd;

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
            send_CW(this_char);
            printf("%c",this_char);
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


// ***********Normal pause between letters *****************************************************   
//                              A      B      C     D    E      F      G     H     I     J
    const uint32_t morse[] = {0x11D,0x1157,0x45D7,0x457,0x11,0x1175,0x1177,0x455,0x45,0x11DDD,
//                              0      1      2     3    4      5      6     7     8     9
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
       0X4755,0X1155,0X4557,0X11577,0X45777,0X117777,0X10,0X80,0X101D75D,0X4077577,
//       30     31     32     33       34      35     36   37     38      39 
// *********************************************************************************************     
//        ?       /        '        !
       0x4057751,0X11757,0X15DDDD,0X4775D7};
//        40      41       42       43
// *********************************************************************************************     


        this_char = ascii_in;
       
        if (this_char == 32) { // look for space char
            this_char = 36;    // index to Morse char
            }
        else if (this_char == 46) { // look for PERIOD char
            this_char = 38;         // index to Morse char
            }
        else if (this_char == 44) { // look for COMMA char
            this_char = 39;    // index to Morse char
            }
        else if (this_char == 63) { // look for QUESTION char
            this_char = 40;         // index to Morse char
            }
        else if (this_char == 47) { // look for FORWARD SLASH char
            this_char = 41;         // index to Morse char
            }
        else if (this_char == 39) { // look for APOSTROPHY char
            this_char = 42;         // index to Morse char
            }
        else if (this_char == 33) { // look for EXCLAMATION char
            this_char = 43;    // index to Morse char
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
            this_char = 37; // SPACE long
            }      
                
        morse_out = morse[this_char];     // get Morse char from array
        
    while (morse_out != 1) {              // send Morse bits              
        bit_out = (morse_out & masknumber);  // isolate bit to send
        gpio_put(DOTL, !bit_out);   //  Dots need LOW to turn ON  
        gpio_put(D24_PIN, bit_out); //  To audio gate, needs HIGH to gate tone through.
        
        morse_out /= 2;                   // divide by 2 to shift Right
        if (cwd_timer != NULL) {
            xTimerStart(cwd_timer, 0);    // delay dot length 
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
        // Time-out arrived
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
         // Timer timed-out, so continue
        xQueueOverwrite(xQdit, &timer_info);
}
/**
 * @brief Switch Debounce Repeat check of Buffer[0] of the Port Extender. The port extender
 * Buffer[0] monitors eight switches. SW7 pushbutton sends ack to stop blinking of ADC Task Output, 
 * Measures sw state, compares NOW state with PREVIOUS state. If states are different
 * sets count == 0 and looks for two states the same.  It then looks for MIN_COUNT counts
 * in a row or more where NOW and PREVIOUS states are the same. Once switch bounce is not observed
 * the Switch state is passed through a Queue to a function to be used as a control signal.
 */
 void sw_debounce_task(void* unused_arg) {
    uint8_t sw_previous_state = 0b11111111;   // initialize sw_previous_state
    uint8_t sw_state = 0b11111111;            // initialize sw_state
    uint32_t count = 5;               // initialize sw_final_state
    uint8_t pcfbuffer[]={0b11111111,0b11111111};// data buffer, must be two bytes
    
    while (true) {
        // Measure SW and add the LED state
        // to the FreeRTOS xQUEUE if switch has changed
        sw_previous_state = sw_state;
        i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, false);
         sw_state = (pcfbuffer[0] & SW_MASK);
         
       
        if(sw_previous_state == sw_state) {
            if (count < MAX_COUNT) {
                count += 1;
            }
            else {  // reset cout to MIN_COUNT
                count = MIN_COUNT;
             }              //  End if (count < 12)
            vTaskDelay(ms_delay10);  // check switch state every 10 ms
         }
        else  { //  if sw_previous state |= sw_state switch has changed
        
             count = 0;  // Need at least MIN_COUNT consecutive same states
             while(count < MIN_COUNT) {
                sw_previous_state = sw_state;
                i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, false);
                 sw_state = (pcfbuffer[0] & SW_MASK);
                if(sw_previous_state == sw_state){
                     count++;
                }
                 else {
                     count = 0;
                 }
                vTaskDelay(ms_delay10);  // check switch state every 10 ms
            }
            switch0(sw_state);
            switch1(sw_state);
            switch2(sw_state);
            switch3(sw_state);
            switch4(sw_state);
            switch5(sw_state);
            switch6(sw_state);
            switch7(sw_state);
            switchBin3(sw_state);
        }   // end else(sw_previous_state |= sw_state)
        vTaskDelay(ms_delay50);  // check switch state every 50 ms
    }  // End while (true)    
}

/*
 * SwitchBin3(int)
 *  Input Switch state value.  The function will Mask out its Switch value 
 * from sw_state and will send Qmessage to task requiring
 * switch change. The switch value is expected to remain in the gueue until
 * changed.
 * This function creates a switch output of 0 to 7 from Switches 2,3,4 which
 * are used to select which phrase to output.
 */
void switchBin3(uint8_t sw_state) {
    uint8_t now = 0 ;
    
    now = (sw_state & 0B0011100)/4;  // Switches 2,3,4

        xQueueOverwrite(xQbin3, &now);
}


/*
 * Switch0(uint8_t)
 *  Input Switch state value.  The function will Mask out its Switch value 
 * from sw_state and will send Qmessage to task requiring
 * switch change. The switch value is expected to remain in the gueue until
 * changed.
 */
void switch0(uint8_t sw_state) {
    uint8_t now = 0 ;
    
    if(sw_state & 0B00000001){
         now = 1;
     }
     else {
         now = 0;
     }
        xQueueOverwrite(xQsw0, &now);
}

/*
 * Switch1(int)
 *  Input Switch state value.  The function will Mask out its Switch value 
 * from sw_state and will send Qmessage to task requiring
 * switch change. The switch value is expected to remain in the gueue until
 * changed.
 */
void switch1(uint8_t sw_state) {
    uint8_t now = 0 ;
    
    if(sw_state & 0B00000010){
         now = 1;
     }
     else {
         now = 0;
     }
        xQueueOverwrite(xQsw1, &now);
}

/*
 * Switch2(int)
 *  Input Switch state value.  The function will Mask out its Switch value 
 * from sw_state and will send Qmessage to task requiring
 * switch change. The switch value is expected to remain in the gueue until
 * changed.
 */
void switch2(uint8_t sw_state) {
    uint8_t now = 0 ;
    
    if(sw_state & 0B00000100){
         now = 1;
        xQueueOverwrite(xQsw2, &now);
     }
     else {
         now = 0;
        xQueueOverwrite(xQsw2, &now);
     }
}

/*
 * Switch3(int)
 *  Input Switch state value.  The function will Mask out its Switch value 
 * from sw_state and will send Qmessage to task requiring
 * switch change. The switch value is expected to remain in the gueue until
 * changed.
 */
void switch3(uint8_t sw_state) {
    uint8_t now = 0 ;
    uint8_t last = 0;
    
    if(sw_state & 0B00001000){
         now = 1;
     }
     else {
         now = 0;
     }
        xQueueOverwrite(xQsw3, &now);
}
/*
 * Switch4(int)
 *  Input Switch state value.  The function will Mask out its Switch value 
 * from sw_state and will send Qmessage to task requiring
 * switch change. The switch value is expected to remain in the gueue until
 * changed.
 */
void switch4(uint8_t sw_state) {
    uint8_t now = 0 ;
    
    if(sw_state & 0B00010000){
         now = 1;
     }
     else {
         now = 0;
     }
        xQueueOverwrite(xQsw4, &now);
}

/*
 * Switch5(uint8_t)
 *  Input Switch state value.  The function will Mask out its Switch value 
 * from sw_state and will send Qmessage to task requiring
 * switch change. The switch value is expected to remain in the gueue until
 * changed.
 */
void switch5(uint8_t sw_state) {
    uint8_t now = 0 ;
    
    if(sw_state & 0B00100000){
         now = 1;
     }
     else {
         now = 0;
     }
        xQueueOverwrite(xQsw5, &now);
}

/*
 * Switch6(uint8_t)
 *  Input Switch state value.  The function will Mask out its Switch value 
 * from sw_state and will send Qmessage to task requiring
 * switch change. The switch value is expected to remain in the gueue until
 * changed.
 */
void switch6(uint8_t sw_state) {
    uint8_t now = 0 ;
    
    if(sw_state & 0B01000000){ //  0X40
         now = 1;
     }
     else {
         now = 0;
     }

        xQueueOverwrite(xQsw6, &now);
}

/*
 * Switch7(uint8_t)
 *  Input Switch state value.  The function will Mask out its Switch value 
 * from sw_state and will send Qmessage to task requiring
 * switch change. The switch value is expected to remain in the gueue until
 * changed.
 */
void switch7(uint8_t sw_state) {
    uint8_t now = 0 ;
    
    if(sw_state & 0B10000000){ //  0X80
         now = 1;
     }
     else {
         now = 0;
     }
        xQueueOverwrite(xQsw7, &now);
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

    // Configure Seven Seg DOT
    gpio_init(DOTL);
    gpio_disable_pulls(DOTL);  // remove pullup and pulldowns
    gpio_set_dir(DOTL, GPIO_OUT);

    // Configure Seven Seg DOT 
    gpio_init(DOTR);
    gpio_disable_pulls(DOTR);  // remove pullup and pulldowns
    gpio_set_dir(DOTR, GPIO_OUT);

    // Configure ADC
    adc_init();  
    adc_gpio_init(26);   
    adc_select_input(0);

    // Configure GPIO Extender
    pcf8575_init();

    // Configure Seven Segment display
    config_seven_seg();

    // initialize board LED OFF
    gpio_put(PICO_LED_PIN, pico_led_state);
}
 
/*
 * RUNTIME START
 */
int main() {
    uint32_t error_state = 0;
    uint32_t pico_led_state = 0;
    uint8_t now;
    char pph[SPTOOSIZE];
    configure_gpio();
    struct op
    {
        uint32_t sw_number;
        uint32_t sw_state;
    }; 
    struct op sw_info = {0,0};

    
    // todo get free sm
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &blink_program);
    blink_pin_forever(pio, 1, offset, 18, 600);  // approx 600 Hz - GPIO18 is tone output
    
     -
    // label Program Screen
    printf("\x1B[2J");            // Clear Screen
    printf("\x1B[%i;%iH",2,3);    // place curser
    puts("*** CW Sending  Program ***");
    printf("\x1B[%i;%iH",4,3);    // place curser
    puts("**************************************");
    printf("\x1B[%i;%ir",6,15);   // set window top and bottom lines
 
// Timer creates pause between repetition of the CW Text
    cwp_timer = xTimerCreate("CWP_TIMER", 
                            PAUSE_PERIOD,
                            pdFALSE,
                            (void*)PAUSE_TIMER_ID,
                            cwp_timer_fired_callback);
        if (cwp_timer == NULL) {
            error_state  += 1;
            }
            

// Timer creates dot length
    cwd_timer = xTimerCreate("CWD_TIMER", 
                            DOT_PERIOD,
                            pdFALSE,
                            (void*)DOT_TIMER_ID,
                            cwd_timer_fired_callback);
        if (cwd_timer == NULL) {
            error_state  += 1;
            }
            

    // Set up tasks
    // FROM 1.0.1 Store handles referencing the tasks; get return values
    // NOTE Arg 3 is the stack depth -- in words, not bytes

    BaseType_t sw_status = xTaskCreate(sw_debounce_task, 
                                         "SW_TASK", 
                                         256, 
                                         NULL, 
                                         7,     // Task priority
                                         &sw_task_handle);
        if (sw_status != pdPASS) {
           error_state  += 1;
            }
    
    BaseType_t cw_status = xTaskCreate(cw_task, 
                                         "CW_TASK", 
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
   
   // Set up the event queue
    xQpause = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQpause == NULL ) error_state += 1;

    xQdit = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQdit == NULL ) error_state += 1;

    xQshuffle = xQueueCreate(1, sizeof(pph));
    if ( xQshuffle == NULL ) error_state += 1;
     
    xQbin3 = xQueueCreate(1, sizeof(uint8_t));
    if ( xQbin3 == NULL ) error_state += 1;
    
    xQsw0 = xQueueCreate(1, sizeof(uint8_t)); 
    if ( xQsw0 == NULL ) error_state += 1;

    xQsw1 = xQueueCreate(1, sizeof(uint8_t)); 
    if ( xQsw1 == NULL ) error_state += 1;

    xQsw2 = xQueueCreate(1, sizeof(uint8_t)); 
    if ( xQsw2 == NULL ) error_state += 1;

    xQsw3 = xQueueCreate(1, sizeof(uint8_t)); 
    if ( xQsw3 == NULL ) error_state += 1;

    xQsw4 = xQueueCreate(1, sizeof(uint8_t)); 
    if ( xQsw4 == NULL ) error_state += 1;

    xQsw5 = xQueueCreate(1, sizeof(uint8_t)); 
    if ( xQsw5 == NULL ) error_state += 1;

    xQsw6 = xQueueCreate(1, sizeof(uint8_t)); 
    if ( xQsw6 == NULL ) error_state += 1;

    xQsw7 = xQueueCreate(1, sizeof(uint8_t)); 
    if ( xQsw7 == NULL ) error_state += 1;

//    xQphrase5 = xQueueCreate(1, sizeof(uint8_t));
//    if ( xQphrase5 == NULL ) error_state += 1;
    
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
