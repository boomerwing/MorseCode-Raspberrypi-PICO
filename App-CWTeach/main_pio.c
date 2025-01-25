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
 * cd ~/FreeRTOS-Play/build/App-CWwords
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
#define DOT_PERIOD 60
#define SPTOOSIZE 130
#define TONE_FREQ 500
#define TONE_GPIO 17
#define CW_LED_GPIO 2
#define CW_AUDIO_GPIO 20
#define PHRASE 4

/* 
 * GLOBALS
 */
    // FROM 1.0.1 Record references to the tasks
TaskHandle_t cw_task_handle = NULL;
TaskHandle_t phrases_task_handle = NULL;

    // These are the Send CW timers
volatile TimerHandle_t cwp_timer;
volatile TimerHandle_t cwd_timer;

    // These are the inter-task queues
volatile QueueHandle_t xQpause = NULL;
volatile QueueHandle_t xQdit = NULL;
volatile QueueHandle_t xQphrases = NULL;

   // Selection of Texts
    char **pstrings;
    char *pnext_string;
    int phrase_select  = 0;

    int lines = 31;                            

    char p00[] = "Come, all you who are thirsty, come to the waters\0";
    char p01[] = "come, buy and eat! Come, buy wine and milk without money and without cost.\0";
    char p02[] = "Why spend money on what is not bread, and your labor on what\0";
    char p03[] = "does not satisfy? Listen, listen to me, and eat what is good,\0";
    char p04[] = "and your soul will delight in the richest of fare.\0";
    char p05[] = "Give ear and come to me; hear me, that your soul may live.\0";
    char p06[] = "I will make an everlasting covenant with you,\0";
    char p07[] = "my faithful love promised to David.\0";
    char p08[] = "See, I have made him a witness to the peoples, a leader and commander\0";
    char p09[] = "of the peoples. Surely you will summon nations you know not,\0";
    char p10[] = "and nations that do not know you will hasten to you, because of the\0";
    char p11[] = "LORD your God, the Holy One of Israel, for he has endowed you with splendor.\0";
    char p12[] = "Seek the LORD while he may be found; call on him while he is near.\0";
    char p13[] = "Let the wicked forsake his way and the evil man his thoughts.\0";
    char p14[] = "Let him turn to the LORD, and he will have mercy on him,\0";
    char p15[] = "and to our God, for he will freely pardon.\0";
    char p16[] = "For my thoughts are not your thoughts, neither are your ways my ways,\0";
    char p17[] = "declares the LORD. As the heavens are higher than the earth,\0";
    char p18[] = "so are my ways higher than your ways and my thoughts than your thoughts.\0";
    char p19[] = "As the rain and the snow come down from heaven, and do\0";
    char p20[] = "not return to it without watering the earth and making it bud\0";
    char p21[] = "and flourish, so that it yields seed for the sower and bread for\0";
    char p22[] = "the eater, so is my word that goes out from my mouth:\0";
    char p23[] = "It will not return to me empty, but will accomplish what I desire\0";
    char p24[] = "and achieve the purpose for which I sent it.\0";
    char p25[] = "You will go out in joy and be led forth in peace; the mountains and hills\0";
    char p26[] = "will burst into song before you, and all the trees of the field\0";
    char p27[] = "will clap their hands. Instead of the thornbush will grow the pine tree,\0";
    char p28[] = "and instead of briers the myrtle will grow. This will be for the\0";
    char p29[] = "LORD’s renown, for an everlasting sign, which will not be destroyed.\0";

    char *phrases[] = {p00,p01,p02,p03,p04,p05,p06,p07,p08,p09,p10,p11,p12,p13,p14,p15,p16,p17,p18,p19,p20,p21,p22,p23,p24,p25,p26,p27,p28,p29,0x0000};

void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq);

/*
 * FUNCTIONS
 */

/*
 * blink_pin_forever - generate a PIO output square wave to gpio pin
 */

void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (clock_get_hz(clk_sys) / (2 * freq)) - 3;
}

/**  
 * @brief Send a set of Phrases out to CW sender
 * Use xQphrases Queue to pass address of phrase to CW task 
 *
 */

void phrases_task(void* unused_arg) {
    UBaseType_t uxNumberOfQSpaces;
    UBaseType_t uxMessagesWaiting  = 0;
    UBaseType_t xStatus;

   while (true) {
        if (!(uxQueueSpacesAvailable( xQphrases ))){
                vTaskDelay(ms_delay300);  // Break for other tasks
            }
        else {
            xStatus = xQueueSendToFront(xQphrases,phrases[phrase_select],portMAX_DELAY);
            phrase_select++;
            if(phrase_select == lines) phrase_select = 0;
             }  // end of if(..
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
    uint8_t phrase_select  = 0;
    char this_char = 0;
    char last_char = 0;
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op tp_info = {1,1};
    char pph[SPTOOSIZE];
     
    // Selection of Texts
    char **pstrings;
    char *pnext_string; 
    
    uint32_t first_flag  = 1;
    uint32_t timer_flag  = 0;
    
    vTaskDelay(ms_delay300);
    
   while (true) {
        // first check to see if there is a new phrase waiting from phrase select task
          if(uxQueueMessagesWaiting(xQphrases)){
            for(i=0;i<SPTOOSIZE; i++) { // purge pph buffer
                pph[i] = '\0'; // fill buffer with string end char
            }
           xStatus = xQueueReceive(xQphrases,pph, 0);
            pnext_string = pph;  // select string to be TX
         }
         else{
            vTaskDelay(ms_delay300);
         }
        
        i = 0;  // sets up start of TX phrase
        last_char = 0x40;  // dummy value
        while ((pnext_string[i] != '\0') && (i < SPTOOSIZE)){   // step through TX phrase
            this_char = toupper(pnext_string[i]); 
            
            send_CW(this_char);
            printf("%c",this_char);
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
        gpio_put(CW_LED_GPIO, bit_out);         //  Dots need High to turn ON  
        gpio_put(CW_AUDIO_GPIO, bit_out);       //  Output to Audio Gate
        
        morse_out /= 2;                   // divide by 2 to shift Right
        if (cwd_timer != NULL) {
            xTimerStart(cwd_timer, 0);  // define dot length 
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

    // Configure CW_AUDIO_PIN
    gpio_init(CW_AUDIO_GPIO);
    gpio_disable_pulls(CW_AUDIO_GPIO);  // remove pullup and pulldowns
    gpio_set_dir(CW_AUDIO_GPIO, GPIO_OUT);

    // Configure TONE_GPIO for CW out to audio gate
    gpio_init(TONE_GPIO);
    gpio_disable_pulls(TONE_GPIO);  // remove pullup and pulldowns
    gpio_set_dir(TONE_GPIO, GPIO_OUT);

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
                                         1024, 
                                         NULL, 
                                         6, 
                                         &cw_task_handle);
        if (cw_status != pdPASS) {
            error_state  += 1;
            }
            
     BaseType_t phrases_status = xTaskCreate(phrases_task, 
                                         "PHRASES_TASK", 
                                         1024, 
                                         NULL, 
                                         5,     // Task priority
                                         &phrases_task_handle);
        if (phrases_status != pdPASS) {
            error_state  += 1;
            }
   
   // Set up the event queue
    xQpause = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQpause == NULL ) error_state += 1;

    xQdit = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQdit == NULL ) error_state += 1;

    xQphrases = xQueueCreate(1, sizeof(pph));
    if ( xQphrases == NULL ) error_state += 1;
     
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
