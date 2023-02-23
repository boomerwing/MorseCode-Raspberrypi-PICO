/**
 * RP2040 FreeRTOS Shuffle
 * 
 * @copyright 2022, Calvin McCarthy
 * @version   1.0.0
 * @licence   MIT
 *
 */
#include <stdio.h>
#include "main.h"
#include "pico/stdlib.h"
#define DECKSIZE 26
#define MINCOUNT 5
#define MAXCOUNT 12

/*
 * GLOBALS
 */
    // This is the inter-task queue
    volatile QueueHandle_t xQueue1 = NULL;
    volatile QueueHandle_t xQueue2 = NULL;


// FROM 1.0.1 Record references to the tasks
TaskHandle_t sw1_task_handle = NULL;
TaskHandle_t sw2_task_handle = NULL;
TaskHandle_t shuffle_task_handle = NULL;


/*
 * FUNCTIONS
 */

/**
 * @brief Switch Debounce 
 * Repeat check of SW, send result to miso led pin task to stop and start blinking
 * Measures sw state, compares NOW state with PREVIOUS state. If states are different
 * sets count == 0 and looks for two states the same.  It then looks for five or more (MIN_COUNT)
 * in a row or more where NOW and PREVIOUS states are the same. Then Switch state is used
 * as a control signal, passed to an action function by a Queue.
 */
 void sw1_debounce(void* unused_arg) {
    uint32_t now = 1;            // initialize sw1_state
    uint32_t last = 1;   // initialize sw1_previous_state
    uint32_t count = 1;   // initialize sw1_final_state
    
    while (true) {
        // Measure SW and add the LED state
        // to the FreeRTOS xQUEUE
        last = now;
        now = gpio_get(SW1);       
        if(last == now) {
            vTaskDelay(ms_delay50);  // check switch state every 50 ms
        else   //  if previous state |= state switch has changed
            count = 0;
            while(count < 3){
                last = now;
                i2c_read_blocking(i2c0, I2C_ADDR, pcfbuffer, 2, true);
                now = readBit(pcfbuffer[0], P05);
                if(last == now) { // switch has stopped bouncing
                    count++;
                }
                else{ // switch is still bouncing
                    count = 0;
                }
            vTaskDelay(ms_delay5);  // check switch state every 5 ms
            }
        xQueueOverwrite(xQsw1, &sw_state);
    }  // End while (true)    
}
/**
 * @brief Switch Debounce Repeat check of SW, send result to miso led pin task to 
 * stop and start blinking
 * Measures sw state, compares NOW state with PREVIOUS state. If states are different
 * sets count == 0 and looks for two states the same.  It then looks for five or more (MIN_COUNT)
 * in a row or more where NOW and PREVIOUS states are the same. Then Switch state is used
 * as a control signal, passed to an action function by a Queue.
 */
 void sw2_debounce(void* unused_arg) {
    uint32_t sw_state = 1;            // initialize sw1_state
    uint32_t sw_previous_state = 1;   // initialize sw1_previous_state
    uint32_t count = 1;   // initialize sw1_final_state
    
    while (true) {
        // Measure SW and add the LED state
        // to the FreeRTOS xQUEUE
        sw_previous_state = sw_state;
        sw_state = gpio_get(SW2);       
        if(sw_previous_state == sw_state) {
            if (count < MAXCOUNT) {
                count += 1;
            }
            else {  // reset cout to MINCOUNT
                count = MINCOUNT;
             }              //  End if (count < 30)
         }
        else   //  if previous state |= state switch has changed
        {
             count = 0;  // Need at least MINCOUNT consecutive same states
        }   // end if(sw_previous_state == sw_state)

        if (count > MINCOUNT) //  switch has stopped bouncing
            {       
                xQueueOverwrite(xQsw2, &sw_state);
            }  //  End if (count > MINCOUNT ) 
        vTaskDelay(ms_delay50);  // check switch state every 20 ms
    }  // End while (true)    
}

/*  
 * @brief Shuffle deck of size DECKSIZE, 
 *  */

 void shuffle_task(void* unused_arg) {
    uint8_t i = 0;   // useful counter
    uint32_t j = 0;   // 
    uint32_t deck_size = DECKSIZE;  // initialize deck_size variable
    uint32_t sw_state = 1;         // initialize sw1_state
    uint32_t pick_place = 0;        // initialize position of char to be picked
    uint32_t next_pos = 0;          // initialize next_pos in Result Deck
    uint32_t passed_value_buffer = 0;  // initialize Queue input buffer
    char pick_char = 63;            // initialize first char picked
    char deck1[DECKSIZE + 1];       // deck 1 preshuffle array
    char deck2[DECKSIZE + 1];       // deck 2 post-shuffle arrray
    char label[DECKSIZE + 1];       // label arrray
    

//  create random seed with initial Toggle of Switch 3
    
        xQueuePeek(xQsw1, &passed_value_buffer,0);
        sw_state = passed_value_buffer;
         if (sw_state == 0) {
                printf("\x1B[%i;%iH",6,0);
                printf("\x1B[K");  //  clear to end of Line Right
                printf("  ** Set Switch1 UP ** "); 
            while (sw_state == 0){
                j += (rand() % 137 * rand() % 79);
                xQueuePeek(xQsw1, &passed_value_buffer,0);
                sw_state = passed_value_buffer;
            }
        }
            printf("\x1B[%i;%iH",6,0);  // place curser
            printf("\x1B[K");  //  clear to end of Line Right
            printf("  *** Set Switch1 Down then Up ***");
            printf("\x1B[%i;%ir",10,38);  // set top and bottom lines of window
       while (sw_state == 1){
            j += (rand() % 137 * rand() % 79);
            xQueuePeek(xQsw1, &passed_value_buffer,0);
            sw_state = passed_value_buffer;
       }
        while (sw_state == 0){  // look for switch to go to UP to start shuffle
            j += (rand() % 137 * rand() % 79);
            xQueuePeek(xQsw1, &passed_value_buffer,0);
            sw_state = passed_value_buffer;
        }
     srand48(j);
    
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
            printf("  *** Set Switch1 Down then Up for New Shuffle  ***");
            printf("\x1B[H");  //  Home
        while (sw_state == 1){
            xQueuePeek(xQsw1, &passed_value_buffer,0);
            sw_state = passed_value_buffer;
       }
        while (sw_state == 0){  // look for switch to go to UP to start shuffle
            xQueuePeek(xQsw1, &passed_value_buffer,0);
            sw_state = passed_value_buffer;
        }
        
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
            vTaskDelay(ms_delay10);  // Break for other tasks
            }  // end of while(deck_size > 0)
      // setup for next shuffle
        for (i = 0; i < DECKSIZE; i++) {
            deck1[i] = deck2[i];
            deck2[i] = '*';  
            }
        vTaskDelay(ms_delay300);  // Break for other tasks
      
   } //   while(true);
}


/*
 * RUNTIME START
 */
int main() {
    uint32_t error_state = 0;
    uint32_t pico_led_state = 0;

    // Enable STDIO
    stdio_init_all();

    stdio_usb_init();
    // Pause to allow the USB path to initialize
    sleep_ms(2000);
    
     // Configure Switch1
    gpio_init(SW1);
    gpio_pull_up(SW1);  // pullup for switches
    gpio_set_dir(SW1, GPIO_IN);
   
    // Configure Switch2
    gpio_init(SW2);
    gpio_pull_up(SW2);  // pullup for switches
    gpio_set_dir(SW2, GPIO_IN);
   
   // label Program Screen
    printf("\x1B[2J");  // Clear Screen
    printf("\x1B[%i;%iH",2,3);  // place curser
    printf("*** Shuffle Program *****");
    printf("\x1B[%i;%iH",4,3);  // place curser
    printf("**************************************");

    // Set up tasks
    // FROM 1.0.1 Store handles referencing the tasks; get return values
    // NOTE Arg 3 is the stack depth -- in words, not bytes
    BaseType_t sw1_status = xTaskCreate(sw1_debounce, 
                                         "SW1_TASK", 
                                         256, 
                                         NULL, 
                                         6,     // Task priority
                                         &sw1_task_handle);
        if (sw1_status != pdPASS) {
           error_state  += 1;
            }
    
    BaseType_t  sw2_status = xTaskCreate(sw2_debounce, 
                                         "SW2_TASK", 
                                         256, 
                                         NULL, 
                                         5,     // Task priority
                                         &sw2_task_handle);
        if (sw2_status != pdPASS) {
           error_state  += 1;
            }
            
            
     BaseType_t shuffle_status = xTaskCreate(shuffle_task, 
                                         "SHUFFLE_TASK", 
                                         256, 
                                         NULL, 
                                         3,     // Task priority
                                         &shuffle_task_handle);
        if (shuffle_status != pdPASS) {
            error_state  += 1;
            }
   
   // Set up the event queue
    xQsw1 = xQueueCreate(1, sizeof(uint32_t)); 
    if ( xQsw1 == NULL ) error_state += 1;

    xQsw2 = xQueueCreate(1, sizeof(uint32_t)); 
    if ( xQsw2 == NULL ) error_state += 1;

    // Start the FreeRTOS scheduler
    // FROM 1.0.1: Only proceed if no tasks signal error in setup
    if (error_state == 0) {
        vTaskStartScheduler();
    }
    else {   // if tasks don't initialize, light pico board led
    // Configure PICO_LED_PIN for Initialization failure warning
        gpio_init(PICO_LED_PIN);
        gpio_disable_pulls(PICO_LED_PIN);  // remove pullup and pulldowns
        gpio_set_dir(PICO_LED_PIN, GPIO_OUT);
   
        pico_led_state = 1;
        gpio_put(PICO_LED_PIN, pico_led_state);
    }
    
    // We should never get here, but just in case...
    while(true) {
        // NOP
    };
}
