/**
 * RP2040 FreeRTOS Template: Send CW with Timer trigger
 * Timer is monostable, creating DOT duration.
 * Text is entered through the PS2 Keyboard
 * Queue is used for a Ring buffer holding up
 * to 64 Characters.
 * A Real time task scans the keyboard and a task sends the Morse Code
 * A terminal printout of sent characters is provided.
 * Code is added so words are not broken if a space comes close
 * before the word.
 * cd ~/FreeRTOS-CW-Kbd/build/App-Keyer
 * 
 * minicom -b 115200 -o -D  /dev/ttyACM0
 * 
 * built on 
 * @copyright 2022, Tony Smith (@smittytone)
 * @version   1.2.0
 * @licence   MIT
 *pico_arm_cortex_m0plus_gcc.cmake
 */
 
#include <stdio.h>
#include "main.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "../Common/ps2/ps2.h"

#define DOT_PERIOD 65
#define INTERVALS 7
#define SENTENCE 1
#define I2C_ADDR 0x20
#define RING_SIZE 64
#define LINE_LENGTH 64 
#define CW_GPIO  17     // pin 22 for CW Tone output

/*  
 * GLOBALS
 */
    // FROM 1.0.1 Record references to the tasks
TaskHandle_t cw_task_handle = NULL;
TaskHandle_t ringbuffer_in_task_handle = NULL;

    // These are the Send CW timers
// volatile TimerHandle_t cwp_timer;
volatile TimerHandle_t cwd_timer;

    // These are the inter-task queues
volatile QueueHandle_t xQtimerd = NULL;
volatile QueueHandle_t xQring = NULL;



/*
 * FUNCTIONS
 */

/**
 * @brief Send CW, blinking RX LED 
 * cwd_timer sets dot duration
 */
void cw_task(void* unused_arg) {

    uint32_t i  = 1;
    uint32_t number  = 1;
    uint32_t phrase_select  = 1;
    char this_char = 0;
    int line_length = 0;
    
    vTaskDelay(ms_delay300);
   while (true) {
        // first check to see if there is any Character in xQring
         if(uxQueueMessagesWaiting(xQring)){// delay in ps2 function
            xQueueReceive(xQring,&this_char, portMAX_DELAY);
            send_CW(this_char);
 // *****  Print out characters being sent   ***************           
 // *****  Provde for minimal break of words ***************           
			if((this_char == 0x0D) || (this_char == 0x0A)){
				printf("%c %c", 0X0A, 0X0D);  // Print CR LF
				line_length = 0;
			}
			if((line_length > (LINE_LENGTH-7)) && (this_char == 0x20 )){
				line_length = 0;  // reset line_length
				printf("%c %c",0X0A, 0X0D);  // Print LF CR
				printf("%c",this_char);
			}
			else if(line_length < LINE_LENGTH){
	        	line_length++;
				printf("%c",this_char);
			}
			else{
				line_length = 0;  // reset line_length
				printf("%c %c",0X0A, 0X0D);  // Print LF CR
				printf("%c",this_char);
			}
			
         }
 // *****  *********************************  ***************           
        vTaskDelay(ms_delay200);

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
    uint32_t char_count = 0;
    
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    }; 
    struct op td_info = {1,0};

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
//		this_char = toupper(this_char);

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
        gpio_put(DOTL, !bit_out);         //  Dots need LOW to turn ON   D11_P15
//        gpio_put(CW_GPIO, !bit_out);       //  To discrete Transistor gate, needs LOW
        gpio_put(CW_GPIO, bit_out);       //  To CD4010 gate, needs HIGH to gate tone through.
        
        morse_out /= 2;                   // divide by 2 to shift Right
        if (cwd_timer != NULL) {
            xTimerStart(cwd_timer, 0);  // define dot length 
            xQueueReceive(xQtimerd, &td_info, portMAX_DELAY); // xQueueReceive empties queue
        }
    }

} 


/**
 * @brief ringbuffer_in task
 * Repeatedly check for Queue Space available.
 * If space available, look for new character from keyboard.
 * If character is available, put in the BACK of the Queue.
 * If Queue is full don't look for a new character. The character at
 * the front of the queue will be removed by the CW task to be send
 * as a CW Morse coded character and also printed on STDOUT
 */
void ringbuffer_in_task(void* unused_arg) {
    uint32_t i = 0; //  counter
    char out_char;
    char new_char;

   UBaseType_t uxNumberOfQSpaces = 1;
    
       
    while(true){
				uxNumberOfQSpaces = uxQueueSpacesAvailable(xQring);
				if(uxNumberOfQSpaces){
					new_char = get_iso8859_code();  // from ps2.c 
					if(new_char){
						xQueueSendToBack(xQring,&new_char,0);  // ring output refresh
					}
				}
        vTaskDelay(ms_delay150);
				
    }  // End while (true)    
} 



/**
 * @brief Callback actioned when the CW DOT timer fires.  Sends trigger to
 * initiate a new dot interval.
 *
 * @param timer: The triggering timer.
 */
void cwd_timer_fired_callback(TimerHandle_t timer) {
    struct op
    {
        uint32_t t_number;
        uint32_t t_state;
    };  
    struct op td_info = {2,0};

    if (timer == cwd_timer) {
        // The timer fired so trigger cw ID in cw task
        xQueueOverwrite(xQtimerd, &td_info);
       }
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
    
    stdio_usb_init(); 
    // Pause to allow the USB path to initialize
    sleep_ms(2000);
    
    // Configure PICO_LED_PIN for Initialization failure warning
    gpio_init(PICO_LED_PIN);
    gpio_disable_pulls(PICO_LED_PIN);  // remove pullup and pulldowns
    gpio_set_dir(PICO_LED_PIN, GPIO_OUT);

    // Configure DOTL LED_PIN 15
    gpio_init(DOTL);
    gpio_disable_pulls(DOTL);  // remove pullup and pulldowns
    gpio_set_dir(DOTL, GPIO_OUT);

    // Configure GPIO 17 PIN 22
    gpio_init(CW_GPIO);
    gpio_disable_pulls(CW_GPIO);  // remove pullup and pulldowns
    gpio_set_dir(CW_GPIO, GPIO_OUT);

    // Configure GPIOCLOCK0 to create 500 Hz tone on GPIO 21
    // using System PLL clock source
    clock_gpio_init(D21_P27, 
                    CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                    0X03C000); //   500 Hz

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
    struct op td_info = {0,0};

    configure_gpio();
    
        gpio_put(DOTR, 1);  	//  Turns DOTs off to begin with  
        gpio_put(DOTL, 1); 		// 
        gpio_put(CW_GPIO, 1); 	// 
        
    // label Program Screen
    printf("\x1B[2J");  // Clear Screen
    printf("\x1B[%i;%iH",2,3);  // place curser
    puts("*** CW Keyboard Sending  Program ***");
    printf("\x1B[%i;%iH",4,1);  // place curser
    puts("0*********1*********2*********3*********4*********5*********6*********7*********8");
    printf("\x1B[%i;%ir",6,22);  // set window top and bottom lines


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
                                         8, 
                                         &cw_task_handle);
        if (cw_status != pdPASS) {
            error_state  += 1;
            }
            
    BaseType_t ringbuffer_in_status = xTaskCreate(ringbuffer_in_task, 
                                         "RINGBUFFER_IN_TASK", 
                                         256, 
                                         NULL, 
                                         6,     // Task priority
                                         &ringbuffer_in_task_handle);
        if (ringbuffer_in_status != pdPASS) {
            error_state += 1;
            }
            
    // Set up the event queue
    xQtimerd = xQueueCreate(1, sizeof(td_info)); 
    if ( xQtimerd == NULL ) error_state += 1;

    xQring = xQueueCreate(RING_SIZE, sizeof(char));
    if ( xQring == NULL ) error_state += 1;
     
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
