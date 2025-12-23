 /*
 * RP2040 FreeRTOS Template: Send CW with Timer trigger
 * Timer is monostable, creating delay between dits sent
 * Sends multiple sentences then repeats.
 * 
 * pio function provides a 500 Hz Square wave
 * pio output and CW output are two inputs to a Common Collector NOR Gate
 * A CW LOW (GPIO 20) blocks the PIO Square wave output (GPIO 17)
 * One of six installed phrases is selected by "select_val" in select_phrase_task;
 * 
 * built on code provided by 
 * @copyright 2022, Tony Smith (@smittytone)
 * @version   1.2.0
 * @licence   MIT
 * 
 * cd ~/FreeRTOS-CW-Play/build/App-CWwords
 *
 */
 
#include <stdio.h>
#include "main.h"
#include "pico/stdlib.h"
// ***************** PIO Includes *****************
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "blink.pio.h"
// ***********************************************

#define PAUSE_PERIOD 1000
#define DOT_PERIOD 65
#define SPTOOSIZE 130
#define TONE_FREQ 500
#define TONE_GPIO  17     // pin 22
#define CW_GPIO    20     // pin 26 
#define CW_LED_GPIO 2     // pin  4
#define SW1 21            // pin 27

#define PHRASE 4

/* 
 * GLOBALS
 */
    // FROM 1.0.1 Record references to the tasks
TaskHandle_t cw_task_handle = NULL;
TaskHandle_t phrases_task_handle = NULL;
TaskHandle_t shuffle_phrases_task_handle = NULL;
TaskHandle_t sw1_debounce_task_handle = NULL;
TaskHandle_t latch_task_handle = NULL;

    // These are the Send CW timers
volatile TimerHandle_t cwp_timer;
volatile TimerHandle_t cwd_timer;

    // These are the inter-task queues
volatile QueueHandle_t xQpause = NULL;
volatile QueueHandle_t xQdit = NULL;
volatile QueueHandle_t xQphrases = NULL;
volatile QueueHandle_t xQshufflephrases = NULL;
volatile QueueHandle_t xQsw1 = NULL;
volatile QueueHandle_t xQlatch = NULL;
volatile QueueHandle_t xQlatcharm = NULL;

void switch1(uint8_t);


   // Selection of Texts
    char **pstrings;
    char *pnext_string;
    int phrase_select  = 0;

    int lines = 31;                            

     char p0[] = "ABOUT";
     char p1[] = "CAN";
     char p2[] = "GOOD";
     char p3[] = "LARGE";
     char p4[] = "ON";
     char p5[] = "THAN";
     char p6[] = "WAS";
     char p7[] = "AFTER";
     char p8[] = "COULD";
     char p9[] = "GOT";
     char p10[] = "LAST";
     char p11[] = "ONE";
     char p12[] = "THAT";
     char p13[] = "WE";
     char p14[] = "ALL";
     char p15[] = "COUNTRY";
     char p16[] = "GREAT";
     char p17[] = "LIKE";
     char p18[] = "ONLY";
     char p19[] = "THE";
     char p20[] = "WELL";
     char p21[] = "AN";
     char p22[] = "DAY";
     char p23[] = "HAD";
     char p24[] = "LITTLE";
     char p25[] = "OR";
     char p26[] = "THEIR";
     char p27[] = "WENT";
     char p28[] = "AND";
     char p29[] = "DAYS";
     char p30[] = "HAS";
     char p31[] = "LOOK";
     char p32[] = "OTHER";
     char p33[] = "THEM";
     char p34[] = "WERE";
     char p35[] = "AM";
     char p36[] = "DEBTS";
     char p37[] = "HAVE";
     char p38[] = "MADE";
     char p39[] = "OUR";
     char p40[] = "THEN";
     char p41[] = "WHAT";
     char p42[] = "ARE";
     char p43[] = "DID";
     char p44[] = "HE";
     char p45[] = "MAKE";
     char p46[] = "OUT";
     char p47[] = "THERE";
     char p48[] = "WHEN";
     char p49[] = "AS";
     char p50[] = "DO";
     char p51[] = "HER";
     char p52[] = "MANY";
     char p53[] = "OVER";
     char p54[] = "THESE";
     char p55[] = "WHERE";
     char p56[] = "DOLLARS";
/** 
    int lines = 20;                            

     char p0[] = "THE";
     char p1[] = "AND";
     char p2[] = "FOR";
     char p3[] = "ARE";
     char p4[] = "BUT";
     char p5[] = "USE";
     char p6[] = "OF";
     char p7[] = "TO";
     char p8[] = "IN";
     char p9[] = "IT";
     char p10[] = "IS";
     char p11[] = "BE";
     char p12[] = "AS";
     char p13[] = "AT";
     char p14[] = "SO";
     char p15[] = "WE";
     char p16[] = "NOT";
     char p17[] = "YOU";
     char p18[] = "ALL";
     char p19[] = "ANY";
     char p20[] = "CAN";
     char p21[] = "HE";
     char p22[] = "BY";
     char p23[] = "OR";
     char p24[] = "ON";
     char p25[] = "DO";
     char p26[] = "IF";
     char p27[] = "ME";
     char p28[] = "MY";
     char p29[] = "UP";
     char p30[] = "AN";
     char p31[] = "HAD";
     char p32[] = "HER";
     char p33[] = "WAS";
     char p34[] = "ONE";
     char p35[] = "OUR";
     char p36[] = "GO";
     char p37[] = "NO";
     char p38[] = "US";
     char p39[] = "AM";
     char p40[] = "OF";
     char p41[] = "TO";
     char p42[] = "IN";
     char p43[] = "IT";
     char p44[] = "IS";
     char p45[] = "BE";
     char p46[] = "AS";
*/

    char *phrases[] = {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18,p19,p20,p21,p22,p23,p24,p25,p26,p27,p28,p29, p30,p31,p32,p33,p34,p35,p36,p37,p38,p39,p40,p41,p42,p43,p44,p45,p46,0x00};

   


/*
 * FUNCTIONS
 */

/*
 * blink_pin_forever - generate a PIO output square wave to gpio pin
 */
void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq);

void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (clock_get_hz(clk_sys) / (2 * freq)) - 3;
}

 
/**  
 * @brief Shuffle the order of phrases then send the selected phrase number
 * to phrases task
 * Use xQphrases Queue to pass address of phrase to CW task 
 *
 */

void shuffle_phrases_task(void* unused_arg) {
    UBaseType_t uxNumberOfQSpaces;
    UBaseType_t uxMessagesWaiting  = 0;
    UBaseType_t xStatus;
    uint8_t sw1_buffer = 0;         // initialize Queue input buffer
    uint8_t sw_state = 1;           // initialize Queue input buffer
    uint8_t pick_char;              // initialize first char picked
    uint8_t deck_size;                  
    uint8_t deck1[lines];       // deck 1 preshuffle array
    uint8_t deck2[lines];       // deck 2 post-shuffle arrray
    uint8_t pick_place;         // initialize position of char to be picked
    uint8_t next_pos;           // initialize next_pos in Result Deck
    uint8_t new_phrase;         // initialize new phrase to send
    uint8_t phrase_number;      // initialize new phrase to send
    uint8_t i;                  // initialize next_pos in Result Deck
    uint8_t j = 27;             // initialize next_pos in Result Deck
    
        srand(456789);
        sw_state = 1;
       while(sw_state == 1){
            vTaskDelay(ms_delay50);  // Break for other tasks, while waiting for first button press
            j += (rand()%137 * rand() % 79);
            xQueuePeek(xQsw1,&sw1_buffer, 0);
            sw_state = sw1_buffer;
        }

//      sw1 pressed makes sw_state == 0, which calculsates one random no.
        while (sw_state == 0){
            vTaskDelay(ms_delay50);  // Break for other tasks
            j += (rand() % 137 * rand() % 79);
            xQueuePeek(xQsw1,&sw1_buffer, 0);
            sw_state = sw1_buffer;
        }
        
//      sw1 released makes sw_state == 1, which calculsates random numbers.
        while (sw_state == 1){
            vTaskDelay(ms_delay50);  // Break for other tasks
            j += (rand() % 137 * rand() % 79);
            xQueuePeek(xQsw1,&sw1_buffer, 0);
            sw_state = sw1_buffer;
        }

//      sw1 pressed makes sw_state == 0, which calculsates random nnumbers.
        while (sw_state == 0){  // look for switch to go to UP to start shuffle
            vTaskDelay(ms_delay50);  // Break for other tasks
            j += (rand() % 137 * rand() % 79);
            xQueuePeek(xQsw1,&sw1_buffer,0);
            sw_state = sw1_buffer; // change sw_state to 1
        }

    for(i = 0;i<lines+1;i++){  // initialize count to randomize
        deck1[i] = i;
        deck2[i] = 0;
    }
    
    
   while (true) {
        next_pos = 0;          // ** reset start position of final deck **
        deck_size = lines;
        
        
        while(deck_size) {
            pick_place = (uint8_t)(rand()%deck_size); //  end of the before-after string
            deck2[next_pos]  = deck1[pick_place];
            next_pos++;  // point to next Deck2 insert position
       
            for (i = pick_place; i < lines; i++) { // close source deck after pick
                 deck1[i] = deck1[i+1];
             }

            deck_size = deck_size - 1;   // shorten deck_size after pick and close of orig deck
            vTaskDelay(ms_delay10);  // Break for other tasks
        } // end of while(deck_size > 0)

        for (i = 0; i< lines; i++){
            deck1[i] = deck2[i];
        }
                
        for(i=0;i<lines;i++){
            phrase_number = deck1[i];
            while(!uxQueueSpacesAvailable( xQshufflephrases )){
                vTaskDelay(ms_delay100);  // Break for other tasks
            }
            xStatus = xQueueSendToFront(xQshufflephrases,&phrase_number,portMAX_DELAY);
            vTaskDelay(ms_delay100);  // Break for other tasks
        }

   } //   end while(true);
}


/**  
 * @brief 
 * Accept phrase number from the shufflephrases task.
 * Use the phrase number to select a phrase from the collection of phrases
 * available and active in the global area.
 * Send the selected Phrase out to CW sender
 * Use xQphrases Queue to pass address of phrase to CW task 
 */

void phrases_task(void* unused_arg) {
    UBaseType_t uxNumberOfQSpaces;
    UBaseType_t uxMessagesWaiting  = 0;
    UBaseType_t xStatus;
    uint8_t phrase_number; 

   while (true) {
        while(!(uxQueueSpacesAvailable( xQphrases ))){ // wait for open xQphrases Queue
                vTaskDelay(ms_delay100);  // Break for other tasks
            }
        while(!uxQueueMessagesWaiting(xQshufflephrases)){ // Check for full Queue
                vTaskDelay(ms_delay100);  // Break for other tasks
            }
            //  accept randomized phrase number
            xStatus = xQueueReceive(xQshufflephrases,&phrase_number, portMAX_DELAY); // xQueueReceive empties queue
            phrase_select = phrase_number;
            xStatus = xQueueSendToFront(xQphrases,phrases[phrase_select],portMAX_DELAY);
    }  // end of while(true)
}

/**
 * @brief Send CW, blinking RX LED, switching audio tone ON and OFF 
 *        xQtimer1 triggers TX of phrase sent from Phrases task.
 */
  
void cw_task(void* unused_arg) {
    UBaseType_t uxMessagesWaiting  = 0;
    UBaseType_t xStatus;

    uint8_t i  = 1;
    uint32_t number = 1;
    uint32_t first_flag  = 0;
    uint8_t phrase_select  = 0;
    uint32_t latch_arm;
    uint32_t latch_buffer  = 1;
    uint32_t string_length;
    char this_char = 0;
    char last_char = 0;
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op tp_info = {1,1};
    char pph[SPTOOSIZE];
    char startcmd[] = "PRESS SW BUTTON TWICE TO START\0";

     
    // Selection of Texts
    char **pstrings;
    char *pnext_string; 
    
    uint32_t timer_flag  = 0;
    
    vTaskDelay(ms_delay300);
     latch_arm = 0;
     
        while(uxQueueMessagesWaiting(xQphrases)){
         xStatus = xQueueReceive(xQphrases,&pph, portMAX_DELAY); // xQueueReceive empties queue
        }
       while (true) {
        // first check to see if there is a new phrase waiting from shuffle task
        if( first_flag == 0){
            pnext_string = startcmd;
            xQueueOverwrite(xQlatcharm, &first_flag); // disarm latch
            first_flag = 1;
            
        }
        else{
            vTaskDelay(ms_delay75);  // Break for other tasks
            if(uxQueueMessagesWaiting(xQphrases)){
                xStatus = xQueueReceive(xQphrases,&pph, portMAX_DELAY); // xQueueReceive empties queue
                pnext_string = pph;  // select string to be TX
            }
        }
        string_length = strlen(pnext_string);
        
        i = 0;  // sets up start of TX phrase
        last_char = 0x40;  // dummy value
        while ((pnext_string[i] != '\0') && (i < SPTOOSIZE)){   // step through TX phrase
            this_char = toupper(pnext_string[i]); 
            
            send_CW(this_char);
            printf("%c",this_char);

    // ******************************************************************************************        
            xQueuePeek(xQlatch,&latch_buffer, 0); // Look for Toggle state (Fast or Slow)
            gpio_put(PICO_LED_PIN, latch_buffer);
            if(!latch_buffer) {  // if Toggle High, Send CW Slower
                send_CW(0x10); //  Long Space char after each Char sent to slow
//                send_CW(0x20); //  Space char after each Char sent to slow
            }
    // ******************************************************************************************   
          
            i++;  // point to next character
            
        }  // end of while(next_string[i] ...
        printf("\n");
     
        // Trigger one-shot timer to delay next CW text phrase output
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
            this_char = 43;         // index to Morse char
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
        gpio_put(CW_GPIO, bit_out);       //  Output to Audio Gate
        gpio_put(CW_LED_GPIO, bit_out);         //  Dots need High to turn ON  
        
        morse_out /= 2;                   // divide by 2 to shift Right
        if (cwd_timer != NULL) {
            xTimerStart(cwd_timer, 0);    // define dot length 
            }
         xStatus = xQueueReceive(xQdit, &tdit_info, portMAX_DELAY); // xQueueReceive empties queue
       }

} 


/*
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



/*
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

/**
 * @brief Switch Debounce Repeat check of SW, send result adc task to 
 * show seven segment LED decimal or hex number
 * Measures sw state, compares NOW state with PREVIOUS state. If states are different
 * sets count == 0 and looks for two states the same.  It then looks for five or more (MIN_COUNT)
 * in a row or more where NOW and PREVIOUS states are the same. Then Switch state is used
 * as a control signal, passed to an action function by a Queue.
 */
 void sw1_debounce_task(void* unused_arg) {
    UBaseType_t xStatus;
    UBaseType_t uxNumberOfQSpaces = 1;
    uint8_t now = 1;            // initialize sw1_state
    uint8_t last = 1;   // initialize sw1_previous_state
    uint8_t count = 1;   // initialize sw1_final_state

    while (true) {
        // Measure SW and add the switch state
        // to the FreeRTOS xQUEUE
        last = now; // save last sw_state
        if(gpio_get(SW1)) now = 1;
        else now = 0;
        
        if(last == now) {
            count++;
            if(count >10) count = 3;
            vTaskDelay(ms_delay50);  // check switch state every 50 ms
            }
        else {
             // previous state != sw_state, switch has changed
             count = 0;  // Need at least MIN_COUNT consecutive same states
             while(count <3){
                last = now;  // Need at least MIN_COUNT consecutive same states
                if(gpio_get(SW1)) now = 1;
                else now = 0;
                if(last == now){
                    count++;
                }
                else{
                    count = 0;
                }
                vTaskDelay(ms_delay5);  // check switch state every 50 ms
            }  //  End if (count>3)
            switch1(now);
       }
    }  // End while (true)    
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
    
    if(sw_state){
        now = 1;
     }
     else {
         now = 0;
     }
        xQueueOverwrite(xQsw1, &now);
}

/* 
 * @brief Switch Latch Task 
 * This receives a switch Queue output and makes an ON/OFF Latch
 * It watches for a HiGH to LOW state change of now.  When the Low state
 * is seen it watches for a LOW to HIGH state change then changes the
 * stete of the Latch variable and puts the Latch variable on the 
 * xQlatch Queue.
 */

 void latch_task(void* unused_arg) {

    uint8_t now = 1;
    uint8_t latch = 1;
    
    while (true) {
        // Read switch input
            xQueuePeek(xQsw1,&now, 0);
            while ( !now ) { // now is LOW
                xQueuePeek(xQsw1,&now, 0);
                if ( now ) {  //  now is back to HIGH so Latch changes state
                    if (latch == 0 ) latch = 1;
                    else  latch = 0;
                xQueueOverwrite(xQlatch, &latch);                    
                }  // end if(now)
            vTaskDelay(ms_delay75);  // check switch value every 75 ms
            }  // end while(!now)
    
        vTaskDelay(ms_delay200);  // check adc value every 200 ms
    }  // End while (true)    
}


/*
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

    // Configure CW_LED_PIN
    gpio_init(CW_LED_GPIO);
    gpio_disable_pulls(CW_LED_GPIO);  // remove pullup and pulldowns
    gpio_set_dir(CW_LED_GPIO, GPIO_OUT);

    // Configure CW_PIN
    gpio_init(CW_GPIO);
    gpio_disable_pulls(CW_GPIO);  // remove pullup and pulldowns
    gpio_set_dir(CW_GPIO, GPIO_OUT);

    // Configure SW1 
    gpio_init(SW1);
    gpio_pull_up(SW1);  // pullup for switches
    gpio_set_dir(SW1, GPIO_IN);


    // Configure TONE_GPIO for CW out to audio gate
    gpio_init(TONE_GPIO);
    gpio_disable_pulls(TONE_GPIO);  // remove pullup and pulldowns
    gpio_set_dir(TONE_GPIO, GPIO_OUT);

    // Enable board LED
    gpio_put(PICO_LED_PIN, pico_led_state);  // set initial state to OFF

/**
    // Configure GPIOCLOCK0 to blink LED on GPIO 21 using System PLL clock source
    // Configure TONE_GPIO for clock output tone
    gpio_init(D21_P27);
    gpio_disable_pulls(D21_P27);  // remove pullup and pulldowns
    gpio_set_dir(D21_P27, GPIO_OUT);

     clock_gpio_init(D21_P27, 
                    CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                     0X3C000); //   500 Hz
 */                   

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

    blink_pin_forever(pio, 0, offset, TONE_GPIO, TONE_FREQ);  // gpio 17, approx 500 Hz
    
    
    // label Program Screen
    printf("\x1B[2J");  // Clear Screen
    printf("\x1B[%i;%iH",2,3);  // place curser
    puts("*** CW Sending  Program ***");
    printf("\x1B[%i;%iH",4,3);  // place curser
    puts("**************************************");
    printf("\x1B[%i;%ir",6,18);  // set window top and bottom lines
 
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
                                         "CW_LED_PIN_TASK", 
                                         1024, 
                                         NULL, 
                                         8, 
                                         &cw_task_handle);
        if (cw_status != pdPASS) {
            error_state  += 1;
            }
            
     BaseType_t phrases_status = xTaskCreate(phrases_task, 
                                         "PHRASES_TASK", 
                                         1024, 
                                         NULL, 
                                         7,     // Task priority
                                         &phrases_task_handle);
        if (phrases_status != pdPASS) {
            error_state  += 1;
            }

     BaseType_t shuffle_phrases_status = xTaskCreate(shuffle_phrases_task, 
                                         "SHUFFLE_PHRASES_TASK", 
                                         256, 
                                         NULL, 
                                         6,     // Task priority
                                         &shuffle_phrases_task_handle);
        if (shuffle_phrases_status != pdPASS) {
            error_state  += 1;
            }
   
     BaseType_t latch_status = xTaskCreate(latch_task,             
                                         "LATCH_TASK", 
                                         256, 
                                         NULL, 
                                         5,     // Task priority
                                         &latch_task_handle);
        if (latch_status != pdPASS) {
            error_state  += 1;
            }
             
    BaseType_t sw1_status = xTaskCreate(sw1_debounce_task, 
                                         "SW1_DEBOUNCE_TASK", 
                                         256, 
                                         NULL, 
                                         4,     // Task priority
                                         &sw1_debounce_task_handle);
        if (sw1_status != pdPASS) {
           error_state  += 1;
            }
    
   // Set up the event queue
    xQpause = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQpause == NULL ) error_state += 1;

    xQdit = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQdit == NULL ) error_state += 1;

    xQphrases = xQueueCreate(1, sizeof(pph));
    if ( xQphrases == NULL ) error_state += 1;
     
    xQshufflephrases = xQueueCreate(1, sizeof(uint8_t));
    if ( xQshufflephrases == NULL ) error_state += 1;
     
    xQsw1 = xQueueCreate(1, sizeof(uint8_t)); 
    if ( xQsw1 == NULL ) error_state += 1; 

    xQlatch = xQueueCreate(1, sizeof(uint8_t));
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
        }
}
