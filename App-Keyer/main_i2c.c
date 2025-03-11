/**
 * RP2040 FreeRTOS Keyer Experiments
 * built on 
 * @copyright 2022, Tony Smith (@smittytone)
 * @version   1.2.0
 * @licence   MIT
 * cd ~/FreeRTOS-CW-Play/build/App-Keyer
 * 
 * Program sends CW text Practice phrases selected by Sw0 to SW2.
 * Sw5 selects slow speed or fast speed of characters
 * The seven seg LED display shows the phrase number selected
 * the PICO LED is lit for Slow CW
 * The switches are read by the PCF8575 GPIO Extender
 * CLock output provides the 500 Hz audio tone.
 * An external two input NAND Gate controls the CW tone
 * 
 */
#include <stdio.h>
#include "main.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include "../Common/Seven_Seg_i2c/seven_seg.h"
#include "../Common/PCF8575_i2c/pcf8575i2c.h"

#define PAUSE_PERIOD 1000
#define DOT_PERIOD 68   // about 20 WPM
#define DOT_TIMER_ID    0
#define PAUSE_TIMER_ID  1
#define	DOTR		   14
#define	DOTL		   15


/* 
 * GLOBALS
 */
    // FROM 1.0.1 Record references to the tasks
TaskHandle_t cw_task_handle = NULL;
TaskHandle_t select_phrase_task_handle = NULL;

    // These are the Send CW timers
volatile TimerHandle_t pause_timer;
volatile TimerHandle_t dot_timer;

    // These are the inter-task queues
volatile QueueHandle_t xQpausetimer = NULL;
volatile QueueHandle_t xQdottimer = NULL;
volatile QueueHandle_t xQspeed = NULL;
volatile QueueHandle_t xQphrase = NULL;



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
    uint32_t speed_select  = 1;
    uint8_t this_char = 0;
    uint8_t last_char = 0;
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op t1_info = {1,1};
     
    // Selection of Texts
    char **pstrings;
    char *pnext_string;
/**    
    char p00[] = "AUTVA EUTAH UTMOA UTAAT OUSET UATIV IVEUT AISHV UIUES SAMET MEVIU TAVAU\0";
    char p01[] = "MAUVA AIVIS UAUEV OVTUA VAUAS AVUAV UEATS UEAVS MIVAS SUAVS VITES AUOMI\0";
    char p02[] = "SAUVS HEOAV VIVSU AVAUE IUISE VIVAU MISAV OSTEA MAVIU OTESA HVTAU SEVUI\0";
    char p03[] = "INANE DABEN DIVON TNIBE ESIBN VASBI TONID SHEDV SEDON OBDSI\0";
    char p04[] = "ENABD DBEBI ISHND OBIUD NTODN ADIBD DANBU ANDEB MINAD BADIV\0";
    char p05[] = "ONESD BIDEN EDMNA DASNB SNEBT ADTDS ADNIN HADNB SMDBA MIDOB\0";
    char p06[] = "imoti EMSTI TMSIE ISHES SITOS ISTEM ITIME MESEI EMIES SHEME\0";
*/    
    
    char p00[] = "FAFHR DALIN RLFWE LEFWI DEWTW TROLF FIFAL FRFIL SBRTW NRWTJ\0";
    char p01[] = "SFOWH IRELT SBLWJ JBAFR BSJWU JAMBR WIENH IJFWS RWJBF FRWJL\0";
    char p02[] = "REROH RASIB JOLIR TRILF ITEMF LFRMB FLRRF JBRLF\0";
    char p03[] = "HEWSP MIWOJ SUEJT BADIJ OBPSI WIDJE ESIBW JABIP\0";
    char p04[] = "MIWAP OBIJD SEJOP TWIEP SHEDJ ISHJW ANPEW EJMPD\0";
    char p05[] = "WISJP JEPAW WEPBJ BOSVP JASIB IWATE EWADP EJIDB\0";
    char p06[] = "PJPWJ PWWPJ NTOWJ SMWJA DIJOP HAWNB WEPJT ODBWS\0";
    
/**    
    char p00[] = "THE QUICK BROWN FOX JUMPED OVER THE LAZY DOGS BACK 0123456789\0";
    char p01[] = "MOIST THEME MOTTO HOIST THOSE THESE SHOOT SHOES SHEET THEME\0";
    char p02[] = "SMOTE SMITE SOOTH TOTEM TOOTH TITHE MITES METES MISTS MOOSE\0";
    char p03[] = "MOTET SISOM SMITT STOOM MOOTS SHOTS SMITH MEETS TEETH TESTS\0";
    char p04[] = "OSTEI SOMET IOHOH TIHME OMEST ISIME HSMOI TMEHI EIMSO MOTIS\0";
    char p05[] = "STOMO SMIEI HEMTO IMSHE IMISI OESIM ESITE MOTHM hitem etihm\0";
    char p06[] = "imoti EMSTI TMSIE ISHES SITOS ISTEM ITIME MESEI EMIES SHEME\0";
*/

/**
    char p00[] = "TKCKG FLUFG GFKRG KUART CVKCL CEGIK GURKE CRUFE LFURG LOGUK KLGCK\0";
    char p01[] = "FLKGC LOGOK GVGEU KURFK RFVUK LKFGC GKLRF KUKUE CGKRU RVCLF CLGKF\0";
    char p02[] = "GVKLR KURIG GUGGK VOTSG KUOGK KUACG GKVUC KFCKL RGFKV GRUKC KGIKC\0";
    char p03[] = "YOYOP QUCHT ZADNY SGYEH JAWYZ XLFRX ZOXSH FETYQ HETOP FLUGX ZEXIZ\0";
    char p04[] = "CRYQG YESTY XONIC ZUZPC GVUBZ PLAZR PURXC XAXXA ХІМР5 WAZIW ISYGL\0";
    char p05[] = "DALQP JAZIJ TOPIG XTZEX BJKRU QTYIX VAXIU BUXIZ SHXYZ CEZIG FZRVQ\0";
    char p06[] = "TCMGQ QGFLX GVGZX QYTXZ ҮҮРЕЕ QLYTM ZEXIZ YILOX GIRFL FZRVQ\0";
*/ 
   
    char pph[] = "hitem etihm imoti CQ CQ CQ K\0";
    char *phrases[8] = {p00,p01,p02,p03,p04,p05,p06,pph};
    pstrings = phrases;  // transmit just one string pointed to by ptr
    
    vTaskDelay(ms_delay300);
    uint32_t first_flag  = 1;
    uint32_t timer_flag  = 0;
    
    
   while (true) {
         if(timer_flag == 0) {  // Timer has triggered timeout
            xQueuePeek(xQphrase, &phrase_select, portMAX_DELAY); 
            pnext_string = phrases[phrase_select];  // select string to be TX
            i = 0;  // sets up start of TX phrase
            last_char = 0x40;  // dummy value
            while (pnext_string[i] != 0) {   // step through TX phrase
                this_char = pnext_string[i]; 
                          
                send_CW(this_char);
                this_char = toupper(this_char);
                printf("%c",this_char);
             //  Speed_select SW (6) HIGH == Slow  ****************
                xQueuePeek(xQspeed, &speed_select, portMAX_DELAY); 
                if(speed_select) send_CW(0x20);  // add a SPACE CHAR
             // ***************************************************                
                i += 1;  // point to next character
                last_char = this_char;  // to correct "short" SPACE Char
                }  // end of while(next_string[i] ...
            printf("\n");
     
                // Start the monostable timer
            } // end of if(timer_flag == 0) ...
        // Trigger one-shot timer to delay next CW text output
        if (pause_timer != NULL) xTimerStart(pause_timer, 0);
        xQueueReceive(xQpausetimer, &t1_info, portMAX_DELAY); // xQueueReceive empties queue
        timer_flag = t1_info.t_state;

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
void send_CW(const char ascii_in) {
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

    
// ***********Normal pause between letters *********************************************   
//                              A      B      C     D    E      F      G     H     I
    const uint32_t morse[] = {0x11D,0x1157,0x45D7,0x457,0x11,0x1175,0x1177,0x455,0x45,
//                              0      1      2     3    4      5      6     7     8
// *************************************************************************************     
//         J     K      L     M     N      O      P      Q      R     S     T 
       0x11DDD,0X11D7,0X115D,0X477,0X117,0X4777,0X22DD,0X11D77,0X45D,0X115,0X47,
//        9     10     11    12    13     14     15     16     17    18    19
// *************************************************************************************     
//        U      V      W     X       Y       Z       0       1        2       3  
       0X475,0X11D5,0X11DD,0X4757,0X11DD7,0X4577,0X477777,0x11DDDD,0X47775,0X11DD5,
//       20     21     22    23      24      25      26      27       28      29  
// *************************************************************************************     
//        4      5      6      7        8       9     SP  SPlg    .        , 
       0X4755,0X1155,0X4557,0X11577,0X45777,0X117777,0X10,0X80,0X101D75D,0X4077577,
//       30     31     32     33       34      35     36   37     38      39 
// *************************************************************************************     
//        ?       /        '        !
       0x4057751,0X11757,0X15DDDD,0X4775D7};
//        40      41       42       43
// *************************************************************************************     


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
        gpio_put(DOTR, !bit_out);  //  Dots need LOW to turn ON  
        gpio_put(DOTL, !bit_out);  // 
        
        morse_out /= 2;                   // divide by 2 to shift Right
        if (dot_timer != NULL) xTimerStart(dot_timer, 0);  // define dot length 
        xQueueReceive(xQdottimer, &t1_info, portMAX_DELAY); // xQueueReceive empties queue
        }

} 


/**
 * @brief Callback actioned when the CW timer fires.  Sends trigger to
 * initiate a new CW String TX.
 *
 * @param timer: The triggering timer.
 */
void pause_timer_fired_callback(TimerHandle_t timer) {
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op timer_info = {1,0};

    if (timer == pause_timer) {
        // The timer fired so trigger cw ID in cw task
        xQueueOverwrite(xQpausetimer, &timer_info);
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
        // The timer fired so trigger cw ID in cw task
        xQueueOverwrite(xQdottimer, &timer_info);
       }
}

/** 
 * @brief Read SW1, SW2 and SW3, calculate binary value, 
 * as a control signal, passed to an action function by a Queue. 
 * ADC reading divided into segments. Value of segments displayed on
 * a Seven Segment LED module
 * Sw0,Sw1,Sw2 select which phrase is to be output.  Sw5 selects whether
 * the phrase will be sent full speed, or if it is to be sent with a space
 * between each character
 */

 void select_phrase_task(void* unused_arg) {  // selects which Text phrase to o/p

    uint8_t  pcfbuffer[2]={0b11111111,0b11111111};// data buffer, must be two bytes
    const uint8_t  phrase_mask = 0b00000111;
    const uint8_t  slow_mask = 0b00100000;
    uint32_t temp_sw_input;
    uint32_t temp_sw_input2;
    uint32_t select_phrase;
    uint32_t select_speed;
   
    while (true) {
        // Read Switches on i2c ports 0, 1, and 2 and add the LED state
        // to the FreeRTOS xQUEUE
      i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, false);
      temp_sw_input = pcfbuffer[0]; // protect input values from change
      temp_sw_input2 = pcfbuffer[0]; // protect input values from change
      select_phrase = (temp_sw_input &= phrase_mask);
      select_speed = (temp_sw_input2 &= slow_mask)/32;

      show_seven_seg(select_phrase);
      gpio_put(PICO_LED_PIN, select_speed);   // LED ON for Slow TX   
    
      xQueueOverwrite(xQphrase, &select_phrase);  // Phrase select
      xQueueOverwrite(xQspeed, &select_speed);  // Speed select

      vTaskDelay(ms_delay300);  // 
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

    // Configure DOTL_PIN for Initialization failure warning
    gpio_init(DOTL);
    gpio_disable_pulls(DOTL);  // remove pullup and pulldowns
    gpio_set_dir(DOTL, GPIO_OUT);

    // Configure DOTR_PIN for Initialization failure warning
    gpio_init(DOTR);
    gpio_disable_pulls(DOTR);  // remove pullup and pulldowns
    gpio_set_dir(DOTR, GPIO_OUT);

    // Configure Seven Segment LED with two dots
    config_seven_seg();
    
    // Enable board LED
    gpio_put(PICO_LED_PIN, pico_led_state);  // set initial state to OFF

    // Configure GPIOCLOCK0 to blink LED on GPIO 21 using System PLL clock source
    clock_gpio_init(D21_P27, 
                    CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                    0X03C000); //   500 Hz
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

    clocks_init();
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
    pause_timer = xTimerCreate("PAUSE_TIMER", 
                            PAUSE_PERIOD,
                            pdFALSE,
                            (void*)PAUSE_TIMER_ID,
                            pause_timer_fired_callback);
        if (pause_timer == NULL) {
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
    xQpausetimer = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQpausetimer == NULL ) error_state += 1;

    xQdottimer = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQdottimer == NULL ) error_state += 1;

    xQspeed = xQueueCreate(1, sizeof(sw_info)); 
    if ( xQspeed == NULL ) error_state += 1; 

    xQphrase = xQueueCreate(1, sizeof(uint32_t));
    if ( xQphrase == NULL ) error_state += 1;
    
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
