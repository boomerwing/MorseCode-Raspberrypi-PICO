/**
 * RP2040 FreeRTOS Template: Send CW with Timer trigger
 * Timer is monostable, creating delay between text sent
 * Sending Morse Code Random Shuffled text
 * Text selected by value read in by Switches 00,01,02
 * Switch 06 operating as a Latch Switch chooses 
 * Normal Code Speed and Slow Code Speed
 * Normal has space between characters 7 dits long
 * and Slow has space between characters 11 dits long
 * There are two methods to create the Audio square wave, Use a PIO
 * or a system clock output.
 * PIO clock pin 24
 * system clock pin 27
 * A two MosFET AND gate provides the audio/CW signal.
 * cd ~/FreeRTOS-CW-Play/build/App-CW_Shuffle
 * 
 * built on 
 * smittytone/Raspberry Pi RP2040 FreeRTOS baseline development project
 * @copyright 2022, Tony Smith (@smittytone)
 * @version   1.2.0
 * @licence   MIT
 */
 
#include <stdio.h>
#include "main.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "../Common/Seven_Seg_i2c/seven_seg.h"
#include "../Common/PCF8575_i2c/pcf8575i2c.h"


#define INTERVALS 7
#define PAUSE_PERIOD 1000
#define DOT_PERIOD 66  // 65 is about 20 WPM
#define SPTOOSIZE 80
#define MIN_COUNT 4
#define MAX_COUNT 10
#define SW_MASK 0B11111111
#define TONE_GPIO  18     // pin 21 for pio Tone output
#define CW_GPIO  17     // pin 22 for CW Tone output

/* 
 * GLOBALS
 */
    // FROM 1.0.1 Record references to the tasks
TaskHandle_t cw_task_handle = NULL;
TaskHandle_t phrases_task_handle = NULL;
TaskHandle_t sw_task_handle = NULL;
TaskHandle_t sw6_latch_task_handle = NULL;
// TaskHandle_t select_phrase_task_handle = NULL;

    // These are the Send CW timers
volatile TimerHandle_t cwp_timer;
volatile TimerHandle_t cwd_timer; 

    // These are the inter-task queues
volatile QueueHandle_t xQpause = NULL;
volatile QueueHandle_t xQdit = NULL;
volatile QueueHandle_t xQphrases = NULL;
volatile QueueHandle_t xQlatch = NULL;
volatile QueueHandle_t xQsw7 = NULL;
volatile QueueHandle_t xQsw6 = NULL;
volatile QueueHandle_t xQsw5 = NULL;
volatile QueueHandle_t xQsw4 = NULL;
volatile QueueHandle_t xQsw3 = NULL;
volatile QueueHandle_t xQsw2 = NULL;
volatile QueueHandle_t xQsw1 = NULL;
volatile QueueHandle_t xQsw0 = NULL;
volatile QueueHandle_t xQbin3 = NULL;
volatile QueueHandle_t xQlatcharm = NULL;

/*
 * FUNCTIONS
 */
void switchbin3(uint8_t);
void switch0(uint8_t);
void switch1(uint8_t);
void switch2(uint8_t);
void switch3(uint8_t);
void switch4(uint8_t);
void switch5(uint8_t);
void switch6(uint8_t);
void switch7(uint8_t);

    uint32_t phrase_select  = 0;
   // Selection of Texts
    char **pstrings;
    char *pnext_string;

/**
   const int lines = 8;                            

    char p00[] = "ABOUT CAN GOOD LARGE ON THAN WAS AFTER COULD GOT LAST ONE THAT WE\0";
    char p01[] = "ALL COUNTRY GREAT LIKE ONLY THE WELL AN DAY HAD LITTLE OR THEIR WENT\0";
    char p02[] = "AND DAYS HAS LOOK OTHER THEM WERE AM DEBTS HAVE MADE OUR THEN WHAT\0";
    char p03[] = "ARE DID HE MAKE OUT THERE WHEN AS DO HER MANY OVER THESE WHERE DOLLARS\0";
    char p04[] = "HERE MAY PEOPLE THEY WHICH BACK DOWN HIM ME PUBLIC THIS WHILE EVEN HIS\0";
    char p05[] = "MORE SAID THROUGH WHO BECAUSE EVERY I MOST SAW TIME WILL BEEN FIRST IF MUST\0";
    char p06[] = "SEE TO WITH BEFORE FOR IN MY SHE TODAY WOULD BEING FOUND INTEREST NEW SINCE\0";
    char p07[] = "TWO YEARS BETWEEN FROM INTO NO SO UNDER YOU BIG GENERAL IS NOT SOME UP YOUR\0";
                                
    char *phrases[] = {p00,p01,p02,p03,p04,p05,p06,p07};
*/
/** 
    int lines = 8;
                            
    const char p00[] = "THAT THAT THAT WITH WITH WITH HAVE HAVE HAVE THIS THIS THIS\0";
    const char p01[] = "WILL WILL WILL THEM THEM THEM YOUR YOUR YOUR FROM FROM FROM\0";
    const char p02[] = "THEY THEY THEY KNOW KNOW KNOW WANT WANT WANT WELL WELL WELL\0";
    const char p03[] = "BEEN BEEN BEEN GOOD GOOD GOOD MUCH MUCH MUCH SOME SOME SOME\0";
    const char p04[] = "TIME TIME TIME WERE WERE WERE VERY VERY VERY WHEN WHEN WHEN\0";
    const char p05[] = "COME COME COME HERE HERE HERE JUST JUST JUST LIKE LIKE LIKE\0";
    const char p06[] = "LONG LONG LONG MAKE MAKE MAKE MANY MANY MANY MORE MORE MORE\0";
    const char p07[] = "ONLY ONLY ONLY OVER OVER OVER SUCH SUCH SUCH TAKE TAKE TAKE\0";
                                
    char *phrases[] = {p00,p01,p02,p03,p04,p05,p06,p07};
*/
/**
    int lines = 12;                            

    char p00[] = "THE THE THE AND AND AND FOR FOR FOR ARE ARE ARE BUT BUT BUT USE USE USE\0";
    char p01[] = "OF OF TO TO IN IN IT IT IS IS BE BE AS AS AT AT SO SO WE WE\0";
    char p02[] = "NOT NOT NOT YOU YOU YOU ALL ALL ALL ANY ANY ANY CAN CAN CAN\0";
    char p03[] = "HE HE BY BY OR OR ON ON DO DO IF IF ME ME MY MY UP UP AN AN\0";
    char p04[] = "HAD HAD HAD HER HER HER WAS WAS WAS ONE ONE ONE OUR OUR OUR\0";
    char p05[] = "GO GO NO NO US US AM AM OF OF TO TO IN IN IT IT IS IS BE BE AS AS\0";
    char p06[] = "OUT OUT OUT DAY DAY DAY GET GET GET HAS HAS HAS HIM HIM HIM\0";
    char p07[] = "AT AT SO SO WE WE HE HE BY BY OR OR ON ON DO DO IF IF ME ME MY\0";
    char p08[] = "HIS HIS HIS HOW HOW HOW MAN MAN MAN NEW NEW NEW NOW NOW NOW\0";
    char p09[] = "OLD OLD OLD SEE SEE SEE TWO TWO TWO WAY WAY WAY WHO WHO WHO\0";
    char p0A[] = "MY UP UP AN AN GO GO NO NO US US AM AM BOY BOY BOY DID DID DID\0";
    char p0B[] = "ITS ITS ITS LET LET LET PUT PUT PUT SAY SAY SAY SHE SHE SHE TOO TOO TOO\0";
                                
    char *phrases[] = {p00,p01,p02,p03,p04,p05,p06,p07,p08,p09,p0A,p0B};
*/

    int lines = 3;                            

    char p00[] = "THE AND FOR ARE BUT USE\0";
    char p01[] = "OF TO IN IT IS BE AS AT SO WE \0";
    char p02[] = "NOT YOU ALL ANY CAN\0";

    char *phrases[] = {p00,p01,p02};


/**  
 * @brief Send a set of Phrases out to CW sender
 * Use xQphrases Queue to pass address of phrase to CW task 
 *
 */

void phrases_task(void* unused_arg) {
    UBaseType_t uxNumberOfQSpaces;
    UBaseType_t uxMessagesWaiting  = 0;
    UBaseType_t xStatus;
    char *new_phrase;

   while (true) {
       new_phrase = phrases[phrase_select];
        if(uxQueueSpacesAvailable( xQphrases )){
            xStatus = xQueueSendToFront(xQphrases,new_phrase,portMAX_DELAY);
            phrase_select++;
            if(phrase_select == lines) phrase_select = 0;
             }  // end of if(..
        vTaskDelay(ms_delay300);  // Break for other tasks
    }  // end of while(true)
}



/**
 * @brief Send CW, blinking LED and sending signal to gate sounding tone
 *        xQtimer1 triggers TX of phrase chosen by ADC input value
 */
void cw_task(void* unused_arg) {
    UBaseType_t uxMessagesWaiting  = 0;
    UBaseType_t xStatus;

    uint32_t number  = 1;
    uint32_t first_flag  = 0;
    uint32_t latch_arm;
    uint32_t string_length;
    uint8_t i  = 1;
    uint8_t latch_buffer  = 1;
    char this_char = 0;
    char last_char = 0;
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op tp_info = {1,1};
     
    char pph[SPTOOSIZE];
    char startcmd[] = "PRESS SW6 BUTTON TWICE TO START\0";

    // Selection of Texts
    char **pstrings;
    char *pnext_string;
    
    uint32_t timer_flag  = 0;
    
    latch_arm = 0;
    vTaskDelay(ms_delay300);
    
       while (true) {
        // first check to see if there is a new phrase waiting from shuffle task
        if( first_flag == 0){
            pnext_string = startcmd;
            xQueueOverwrite(xQlatcharm, &latch_arm); // set latch OFF
            first_flag = 1;
            
        }
        else{
            vTaskDelay(ms_delay75);  // Break for other tasks
            xStatus = xQueueReceive(xQphrases,&pph, portMAX_DELAY); 
            pnext_string = pph;  // select string to be TX
        }
        string_length = strlen(pnext_string);
        
         i = 0;  // sets up start of TX phrase
        while (i < string_length) {   // step through TX phrase
            this_char = pnext_string[i]; 
  
            send_CW(this_char);
            printf("%c",this_char);  // Print char after sounding CW character
            i++;  // point to next character
        }  // end of while(next_string[i] ...
        printf("\n");
        
            for(i=0;i<SPTOOSIZE; i++) { // purge pph buffer
                pph[i] = '\0'; // fill buffer with string end char
            }
            if(!first_flag){
                latch_arm = 1;  
                xQueueOverwrite(xQlatcharm, &latch_arm); // latch set to ACTIVE after prompt acknowledge
             }

             // Trigger one-shot timer to delay next CW text output
        if (cwp_timer != NULL) xTimerStart(cwp_timer, 0);

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
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op tdit_info = {1,1};



// **** Full Speed  with punctuation ***********************************************************     
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
       0X4755,0X1155,0X4557,0X11577,0X45777,0X117777,0X10,0X100,0X11D75D,0X477577,
//       30     31     32     33       34      35     36   37     38      39 
// *********************************************************************************************     
//        ?       /        '       !
       0x45775,0X11757,0X15DDDD,0X4775D7};
//        40      41       42      43
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
        else if (this_char == 33) { // look for Exclamation char
            this_char = 43;    // index to Morse char
            }
        else if (this_char == 0x10) { // look for LONG SPACE char
            this_char = 37;    // index to Morse char
            }
        else if (this_char == 0x20) { // look for SPACE char
            this_char = 36;    // index to Morse char
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
            this_char = 36; // SPACE for any unidentified characters
         }   
        morse_out = morse[this_char];     // get Morse char from array
        
//  Code for FET Output ****************************************************
    while (morse_out != 1) {              // send Morse bits              
        bit_out = (morse_out & masknumber);  // isolate bit to send
        gpio_put(DOTL, !bit_out);         //  Dots need LOW to turn ON   D11_P15
        gpio_put(CW_GPIO, !bit_out);       //  To audio gate, needs LOW to gate tone through.
        
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
/* 
 * @brief Switch Latch Task 
 * This receives a switch Queue output and makes an ON/OFF Latch
 * It watches for a HiGH to LOW state change of now.  When the Low state
 * is seen it watches for a LOW to HIGH state change then changes the
 * stete of the Latch variable and puts the Latch variable on the 
 * xQlatch Queue.
 * The task is armed with a TRUE message on xQlatcharm so it can share the
 * same Switch input as the Prompt Acknowledge (PA) action.  When the PA
 * action is not complete (Two switch presses) the latch is not active.
 * When the PA action is complete the PA action is never needed again and
 * the switch can be given the job of changing the Code speed.
 */

 void sw6_latch_task(void* unused_arg) {

    uint8_t now = 1;
    uint8_t latch = 1;
    uint32_t latch_arm;
    
    while (true) {
//   First check to see if the Start Prompt has been sent and acknowledged 
        xQueuePeek(xQlatcharm,&latch_arm, 0);

        if(latch_arm){  // if latch_arm is TRUE, latch will be active.
        // Read switch input
            xQueuePeek(xQsw6,&now, 0);
            while ( !now ) { // now is LOW
                xQueuePeek(xQsw6,&now, 0);
                if ( now ) {  //  now is back to HIGH so Latch changes state
                    if (latch == 0 ) latch = 1;
                    else  latch = 0;
                    xQueueOverwrite(xQlatch, &latch);                    
                }  // end if(now)
                vTaskDelay(ms_delay75);  // check switch value every 75 ms
            }  // end while(!now)
        }  // End if(latch_arm)
        vTaskDelay(ms_delay200);  // check adc value every 200 ms
    }  // End while (true)    
}

/**
 * @brief Switch Debounce Repeat check of Buffer[0] of the Port Extender. The port extender
 * Buffer[0] monitors eight switches. 
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
            switchbin3(sw_state);

        }   // end else(sw_previous_state |= sw_state)
        vTaskDelay(ms_delay100);  // check switch state every 50 ms
    }  // End while (true)    
}
/*
 * Switchbin3(uint8_t)
 *  Input Switch state value.  The function will Mask out its Switch value 
 * from sw_state and will send Qmessage to task requiring
 * switch change. The switch value is expected to remain in the gueue until
 * changed.
 */
void switchbin3(uint8_t sw_state) {
    uint8_t now = 0 ;
    uint32_t out = 0;
    
    now = (sw_state & 0B00000111);
    out = (uint32_t) now;
    xQueueOverwrite(xQbin3, &out);
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
        xQueueOverwrite(xQsw6, &now);
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
    gpio_init(CW_GPIO);
    gpio_disable_pulls(CW_GPIO);       // remove pullup and pulldowns
    gpio_set_dir(CW_GPIO, GPIO_OUT);

    // Configure CW GPIO
    gpio_init(DOTL);
    gpio_disable_pulls(DOTL);          // remove pullup and pulldowns
    gpio_set_dir(DOTL , GPIO_OUT);

    // Configure Seven Seg DOT 
    gpio_init(DOTR);
    gpio_disable_pulls(DOTR);          // remove pullup and pulldowns
    gpio_set_dir(DOTR, GPIO_OUT);

    // Configure TONE_GPIO for clock output tone
    gpio_init(D21_P27);
    gpio_disable_pulls(D21_P27);  // remove pullup and pulldowns
    gpio_set_dir(D21_P27, GPIO_OUT);

/**
 *     // Configure TONE_GPIO for PIO Tone
    gpio_init(TONE_GPIO);
    gpio_disable_pulls(TONE_GPIO);  // remove pullup and pulldowns
    gpio_set_dir(TONE_GPIO, GPIO_OUT);
*/

    // Configure GPIOCLOCK0 to blink LED on GPIO 21 using System PLL clock source
    clock_gpio_init(D21_P27, 
                    CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                     0X3C000); //   500 Hz
                    
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
/**
    // This PIO code starts square wave to create Tone for CW. Use this OR system clock    
    // todo get free sm
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &blink_program);
    blink_pin_forever(pio, 1, offset, 18, 500);  // approx 600 Hz - GPIO18 is tone output
  */  
     -
    // label Program Screen
    printf("\x1B[2J");  // Clear Screen
    printf("\x1B[%i;%iH",2,3);  // place curser
    puts("*** CW Sending  Program ***");
    printf("\x1B[%i;%iH",4,1);  // place curser
    puts("0****^****1****^****2****^****3****^****4****^****5****^*****6****^****7****^*****8");
    printf("\x1B[%i;%ir",6,15);  // set window top and bottom lines
 
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

    BaseType_t cw_status = xTaskCreate(cw_task, 
                                         "CW_TASK", 
                                         256, 
                                         NULL, 
                                         6, 
                                         &cw_task_handle);
        if (cw_status != pdPASS) {
            error_state  += 1;
            }
            
     BaseType_t phrases_status = xTaskCreate(phrases_task, 
                                         "PHRASES_TASK", 
                                         256, 
                                         NULL, 
                                         7,     // Task priority
                                         &phrases_task_handle);
        if (phrases_status != pdPASS) {
            error_state  += 1;
            }
   
    BaseType_t sw_status = xTaskCreate(sw_debounce_task, 
                                         "SW_TASK", 
                                         256, 
                                         NULL, 
                                         5,     // Task priority
                                         &sw_task_handle);
        if (sw_status != pdPASS) {
           error_state  += 1;
            }
    
    BaseType_t latch_status = xTaskCreate(sw6_latch_task,             
                                         "SW6_LATCH_TASK", 
                                         256, 
                                         NULL, 
                                         4,     // Task priority
                                         &sw6_latch_task_handle);
        if (latch_status != pdPASS) {
            error_state  += 1;
            }
             
   // Set up the event queue
    xQpause = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQpause == NULL ) error_state += 1;

    xQdit = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQdit == NULL ) error_state += 1;

    xQphrases = xQueueCreate(1, sizeof(pph));
    if ( xQphrases == NULL ) error_state += 1;
     
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

    xQbin3 = xQueueCreate(1, sizeof(uint8_t)); 
    if ( xQbin3 == NULL ) error_state += 1;

    xQlatch = xQueueCreate(1, sizeof(uint32_t));
    if ( xQlatch == NULL ) error_state += 1;
    
    xQlatcharm = xQueueCreate(1, sizeof(uint32_t)); 
    if ( xQlatcharm == NULL ) error_state += 1;

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
