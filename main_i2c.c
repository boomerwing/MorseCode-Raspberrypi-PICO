   /**
 * RP2040 FreeRTOS Template
 * 
 * @copyright 2022, Tony Smith (@smittytone)
 * @version   1.4.1
 * @licence   MIT
 *  cd ~/FreeRTOS-Play/build/App-Shuffle
 *  
 * Simulate Reading measurement points, display ADC value, look for
 * Measurement alarm boundary.  If measurement is less than boundary,
 * blink seven seg display.  If the measured value moves higher than 
 * alarm boundary, continue blinking until blinking is acknowledged.
 *   - main_i2c-E.C changes dot display to second 
 *     Seven Seg Display, Ports 14 and 15
 */
#include <stdio.h>
#include "main.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "../Common/Seven_Seg_i2c/seven_seg.h"  
#include "../Common/PCF8575-i2c/pcf8575i2c.h"

#define HEX_INTERVALS 0X100
#define DEC_INTERVALS 400
#define MIN_COUNT 4  
#define HEX_ALARM_BOUNDARY 0X08
#define SW_MASK 0B11111111
#define DECKSIZE 26



/*
 * GLOBALS
 */
 
// These are the inter-task queues
volatile QueueHandle_t xQblink = NULL;
volatile QueueHandle_t xQsw7 = NULL;
volatile QueueHandle_t xQsw6 = NULL;
volatile QueueHandle_t xQsw5 = NULL;
volatile QueueHandle_t xQsw4 = NULL;
volatile QueueHandle_t xQsw3 = NULL;
volatile QueueHandle_t xQsw2 = NULL;
volatile QueueHandle_t xQsw1 = NULL;
volatile QueueHandle_t xQsw0 = NULL;

// These are the inter-task queues
volatile TimerHandle_t xTblink = NULL;

// Set a delay time of exactly 500ms
const TickType_t ms_delay = 500 / portTICK_PERIOD_MS;
 
// FROM 1.0.1 Record references to the tasks
TaskHandle_t blink_task_handle = NULL;
TaskHandle_t gpio_led_task_handle = NULL;
TaskHandle_t shuffle_task_handle = NULL;
TaskHandle_t sw_debounce_task_handle = NULL;

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
 
/*  
 * @brief Shuffle deck of size DECKSIZE, 
 *  Note: The priority of this task must be low to allow other tasks time. 
 *  */

 void shuffle_task(void* unused_arg) {
    uint32_t i = 0;   // useful counter
    uint32_t j = 0;   // 
    uint32_t deck_size = DECKSIZE;  // initialize deck_size variable
    uint8_t sw_state = 1;          // initialize sw1_state
    uint32_t pick_place = 0;        // initialize position of char to be picked
    uint32_t next_pos = 0;          // initialize next_pos in Result Deck
    uint8_t sw6_buffer = 0;        // initialize Queue input buffer
    char pick_char = 63;            // initialize first char picked
    char deck1[DECKSIZE + 1];       // deck 1 preshuffle array
    char deck2[DECKSIZE + 1];       // deck 2 post-shuffle arrray
    char label[DECKSIZE + 1];       // label arrray
    UBaseType_t uxMessagesWaiting  = 0;
   

//  create random seed with initial Toggle of Switch 6
    
         if (sw_state == 1) {
                printf("\x1B[%i;%iH",6,0);
                printf("\x1B[K");  //  clear to end of Line Right
                printf("  ** Set Switch 6 Down ** ");
            } 
            while (sw_state == 1){
                j += (rand() % 137 * rand() % 79);
                uxMessagesWaiting = uxQueueMessagesWaiting(xQsw6);
                if(uxMessagesWaiting){
                    xQueuePeek(xQsw6, &sw6_buffer,0);
                    sw_state = sw6_buffer;
                }
            vTaskDelay(ms_delay50);  // Break for other tasks

           }
        
         if (sw_state == 0) {
                printf("\x1B[%i;%iH",6,0);
                printf("\x1B[K");  //  clear to end of Line Right
                printf("  ** Set Switch 6 UP ** ");
            } 
            while (sw_state == 0){
                j += (rand() % 137 * rand() % 79);
                uxMessagesWaiting = uxQueueMessagesWaiting(xQsw6);
                if(uxMessagesWaiting){
                    xQueuePeek(xQsw6, &sw6_buffer,0);
                    sw_state = sw6_buffer;
                }
            vTaskDelay(ms_delay50);  // Break for other tasks
           }

            printf("\x1B[%i;%iH",6,0);  // place curser
            printf("\x1B[K");  //  clear to end of Line Right
            printf("  *** Set Switch 6 Down then Up for New Shuffle ***");

        // label both orig and final decks
            for (i = 0; i < DECKSIZE; i++) {
                j = (i %10) +1;
                if (j == 10) j = 0;
                label[i] = 48 +j;
                }
            label[DECKSIZE]  = '\0';  //  add end to the Label string
            printf("\x1B[%i;%iH",8,0);  //  place curser
            printf("\x1B[K");  //  clear to end of Line Right
            printf("%s  %s", label, label);    //  print out label string
                                         
            printf("\x1B[%i;%ir",10,38);  // set top and bottom lines of window
      srand48(j);
    
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
            printf("\x1B[%i;%iH",6,0);  // place curser
            printf("\x1B[K");  //  clear to end of Line Right
            printf("  *** Set Switch 6 Down then Up for New Shuffle ***");

        while (sw_state == 1){
            uxMessagesWaiting = uxQueueMessagesWaiting(xQsw6);
            if(uxMessagesWaiting){
                xQueuePeek(xQsw6, &sw6_buffer,0);
                sw_state = sw6_buffer;
            }
            vTaskDelay(ms_delay50);  // Break for other tasks
       }
        while (sw_state == 0){  // look for switch to go to UP to start shuffle
            uxMessagesWaiting = uxQueueMessagesWaiting(xQsw6);
            if(uxMessagesWaiting){
                xQueuePeek(xQsw6, &sw6_buffer,0);
                sw_state = sw6_buffer;
            }
            vTaskDelay(ms_delay50);  // Break for other tasks
        }
        printf("\x1B[K");  //  clear to end of Line Right
        printf("\x1B[%i;%iH",10,0);  // place curser
        printf("\x1B[K");  //  clear to end of Line Right
        printf("%s  %s", deck2, deck1);    //  print out start string

        
        deck_size = DECKSIZE;  //  reset start position of original deck
        next_pos = 0;          // ** reset start position of final deck **

            /* deck_size shortens as each random character is removed and the 
             * hole closed. Deck1 shortens as Deck2 is Filled. 
             */
        while(deck_size > 0) {
            pick_place = rand() % deck_size;
            printf("  * %u  ", (pick_place + 1)); //  print place of pick on the
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

            printf("\n%s  %s", deck2, deck1);    //  print out result of one "Pick"
            vTaskDelay(ms_delay20);  // Break for other tasks
            }  // end of while(deck_size > 0)
      // setup for next shuffle
        for (i = 0; i < DECKSIZE; i++) {
            deck1[i] = deck2[i];
            deck2[i] = '*';  
            }

        vTaskDelay(ms_delay100);  // Break for other tasks
      
   } //   end while(true);
}

/**
 * @brief Repeatedly flash the Pico's built-in LED.
 *  Time delay of Flash defined by TaskDelayUntil()
 *  Message from SW0 and SW1 sets blink rate. 
 *  xQblink entry remains in Queue buffer until changed so any task can
 *  have blinking LEDs controlled by the blink_task. 
 */
void blink_task(void* unused_arg) {
    
    uint8_t sw0_buffer = 0;
    uint8_t sw1_buffer = 0;
    uint8_t sw_buffer = 0;
    int steps = 450;
    uint8_t pico_led_state = 0;
    UBaseType_t uxMessagesWaiting  = 0;
    
   // Initialize start time for vTaskDelayUntil
    TickType_t lastTickTime = xTaskGetTickCount();
        
    while (true) {
        uxMessagesWaiting = uxQueueMessagesWaiting(xQsw0);
        if(uxQueueMessagesWaiting){
            xQueuePeek(xQsw0, &sw0_buffer,0);
            }
            
        uxMessagesWaiting = uxQueueMessagesWaiting(xQsw1);
        if(uxQueueMessagesWaiting){
            xQueuePeek(xQsw1, &sw1_buffer,0);
          }      
            
            sw_buffer = (sw1_buffer *2) + sw0_buffer;  // 00, 01, 10, 11
            
            switch(sw_buffer) // four possible blink rates
            {
                case (0b00):  // colon
                    steps = 400;
                    break;
                case (0b01):
                    steps = 600;
                    break;
                case (0b10):
                    steps = 800;
                    break;
                case (0b11):
                    steps = 1000;
                    break;
            }

        pico_led_state = !pico_led_state ;  // Toggle pico led state

        gpio_put(PICO_LED_PIN, pico_led_state); 
        xQueueOverwrite(xQblink, &pico_led_state);
        vTaskDelayUntil(&lastTickTime, pdMS_TO_TICKS(steps));
        }
} 


/**
 * @brief Alternately flash Right and Left Dots on Seven Seg Display
 *        based on the value passed via the inter-task xQblink queue.
 *        
 */
void gpio_led_task(void* unused_arg) {  
    // This variable will take a copy of the value
    // added to the FreeRTOS Queue
    uint8_t blink_buffer = 0;
    UBaseType_t uxMessagesWaiting = 0;
    
    while (true) {
        // Check for an item in the blink Queue
        // Read of Queue does not empty it
        uxMessagesWaiting = uxQueueMessagesWaiting(xQblink);
        if(uxQueueMessagesWaiting){
            xQueuePeek(xQblink, &blink_buffer,0);
        }
        vTaskDelay(ms_delay100);  // check Queue input every 100 ms
        // Received a value so flash DOTL and DOTR accordingly
        gpio_put(DLT_PIN, blink_buffer == 0 ? 0 : 1);
        gpio_put(DRT_PIN, blink_buffer == 1 ? 0 : 1);
   }
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
    uint8_t sw_previous_state = 1;   // initialize sw_previous_state
    uint8_t sw_state = 1;            // initialize sw_state
    uint32_t count = 5;               // initialize sw_final_state
    uint8_t pcfbuffer[]={0b11111111,0b11111111};// data buffer, must be two bytes
    
    while (true) {
        // Measure SW and add the LED state
        // to the FreeRTOS xQUEUE if switch has changed
        sw_previous_state = sw_state;
        i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, false);
         sw_state = (pcfbuffer[0] & SW_MASK);
         
        if(sw_previous_state == sw_state) {
            if (count < 8) {
                count += 1;
            }
            else {  // reset cout to MIN_COUNT
                count = MIN_COUNT;
             }              //  End if (count < 12)
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
        }   // end else(sw_previous_state |= sw_state)
        vTaskDelay(ms_delay50);  // check switch state every 50 ms
    }  // End while (true)    
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
    
    if(sw_state & 0B01000000){
         now = 1;
     }
     else {
         now = 0;
     }
        xQueueOverwrite(xQsw6, &now);
}

/*
 * Switch7(int)
 *  Input Switch state value.  The function will Mask out its Switch value 
 * from sw_state and will send Qmessage to task requiring
 * switch change. The switch value is expected to remain in the gueue until
 * changed.
 */
void switch7(uint8_t sw_state) {
    uint8_t now = 0 ;
    
    if(sw_state & 0B10000000){
         now = 1;
     }
     else {
         now = 0;
     }
        xQueueOverwrite(xQsw7, &now);
}

/**
 * @brief Initialize GPIO Pins for input and output.
 *        Initialize seven segment display
 *        Initialize pcf8575 GPIO Extender
 */
void configure_gpio(void) {
    uint8_t pico_led_state = 0;

    // Configure PICO_LED_PIN for Initialization failure warning
    gpio_init(PICO_LED_PIN);
    gpio_disable_pulls(PICO_LED_PIN);  // remove pullup and pulldowns
    gpio_set_dir(PICO_LED_PIN, GPIO_OUT);
    
    // Configure D6_PIN for led_task_gpio
    gpio_init(DLT_PIN);
    gpio_disable_pulls(DLT_PIN);  // remove pullup and pulldowns
    gpio_set_dir(DLT_PIN, GPIO_OUT);
    
    // Configure D7_PIN for led_task_gpio 
    gpio_init(DRT_PIN);
    gpio_disable_pulls(DRT_PIN);  // remove pullup and pulldowns
    gpio_set_dir(DRT_PIN , GPIO_OUT);


    // Configure GPIO Extender
    pcf8575_init();

    // Configure Seven Segment display
    config_seven_seg();

}


/*
 * RUNTIME START
 */
int main() {
    uint32_t error_state = 0;
    uint8_t pico_led_state = 0;
    
    stdio_usb_init(); 
    // Pause to allow the USB path to initialize
    sleep_ms(2000);
    
    configure_gpio();
    
        // label Program Screen
    printf("\x1B[2J");  // Clear Screen
    printf("\x1B[%i;%iH", 2,3);  // place curser
    printf("*** SHUFFLE Program ***");
    printf("\x1B[%i;%iH",4,2);  // place curser
    printf("**************************************\n");

  
    // Set up tasks
    // FROM 1.0.1 Store handles referencing the tasks; get return values
    // NOTE Arg 3 is the stack depth -- in words, not bytes
      BaseType_t shuffle_status = xTaskCreate(shuffle_task, 
                                         "SHUFFLE_TASK", 
                                         256, 
                                         NULL, 
                                         8,     // Task priority
                                         &shuffle_task_handle);
        if (shuffle_status != pdPASS) {
            error_state  += 1;
            }
   
    BaseType_t blink_status = xTaskCreate(blink_task, 
                                         "BLINK_TASK", 
                                         128, 
                                         NULL, 
                                         7, 
                                         &blink_task_handle);
        if (blink_status != pdPASS) {
            error_state  += 1;
            }
             
    BaseType_t gpio_status = xTaskCreate(gpio_led_task, 
                                         "GPIO_LED_TASK", 
                                         128, 
                                         NULL, 
                                         6, 
                                         &gpio_led_task_handle);
    
        if (gpio_status != pdPASS) {
            error_state  += 1;
            }
             
    BaseType_t sw_status = xTaskCreate(sw_debounce_task, 
                                         "SW_DEBOUNCE_TASK", 
                                         256, 
                                         NULL, 
                                         5,     // Task priority
                                         &sw_debounce_task_handle);
        if (sw_status != pdPASS) {
           error_state  += 1;
            }
    
   // Set up the  queues
     
    xQblink = xQueueCreate(1, sizeof(uint8_t));
    if ( xQblink == NULL ) error_state += 1;

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

     
  // Start the FreeRTOS scheduler
    // FROM 1.0.1: Only proceed with valid tasks
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
