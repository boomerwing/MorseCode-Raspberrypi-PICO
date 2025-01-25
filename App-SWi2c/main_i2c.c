    /**
 * RP2040 FreeRTOS Template
 * 
 * @copyright 2022, Tony Smith (@smittytone)
 * @version   1.4.1
 * @licence   MIT
 *  cd ~/FreeRTOS-Play/build/App-SW  
 *  
 * Simulate Reading measurement points, display ADC value, look for
 * Measurement alarm boundary.  If measurement is less than boundary,
 * blink seven seg display.  If the measured value moves higher than 
 * alarm boundary, continue blinking until blinking is acknowledged.
 *   - main_i2c-E.C changes dot display to second 
 *     Seven Seg Display, Ports 14 and 15
 * Also exercises eight switch entries from the GPIO Extender.  There are
 * individual switch actions and multiple switch actions.  Switch positions
 * are continually available on the appropriate Queue outputs.
 */
#include <stdio.h>
#include "main.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h" 
#include "hardware/pwm.h" 
#include "../Common/Seven_Seg_i2c/seven_seg.h"  
#include "../Common/PCF8575-i2c/pcf8575i2c.h"

#define HEX_INTERVALS 0X100
#define DEC_INTERVALS 400
#define MIN_COUNT 4  
#define ALARM_BOUNDARY 8
#define HEX_ALARM_BOUNDARY 0X0008
#define SW_MASK 0B11111111



/*
 * GLOBALS
 */
 
// These are the inter-task queues
volatile QueueHandle_t xQblink = NULL;
volatile QueueHandle_t xQbin2 = NULL;
volatile QueueHandle_t xQbin3 = NULL;
volatile QueueHandle_t xQbin4 = NULL;
volatile QueueHandle_t xQbin6 = NULL;
volatile QueueHandle_t xQadc = NULL;
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
TaskHandle_t adc_task_handle = NULL;
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
void switchBin2(uint8_t);   // Bits 0,1 of 8 switch group
void switchBin3(uint8_t);   // Bits 2,3,4 of 8 switch group
void switchBin4(uint8_t);   // Bits 2,3,4,5 of 8 switch group
void switchBin6(uint8_t);   // Bits 0,1,2,3,4,5 of 8 switch group
 
/* 
 * @brief Measure Value of ADC Input 0, 
 * Display value of ADC on a seven segment LED module.
 * Set up as a monitoring display. The 7seg display will blink
 * if the ADC input is less than a pre-selected value.  When input returns
 * to acceptable value, blink will continue until it is acknowledged.
 */

void adc_task(void* unused_arg) {

    uint16_t now = 1 ;
    uint16_t measured = 1;
    uint8_t ack_flag = 0;
    uint8_t sw7_state = 0;
    uint8_t sw7_buffer = 0;
    uint8_t blink_buffer = 0;
    const uint8_t blanked = 20 ;
   UBaseType_t uxMessagesWaiting  = 0;
   UBaseType_t xtBlinkStarted  = 0;

    while (true) { 
        // Measure ADC
        now = adc_read();
        xQueueOverwrite(xQadc, &now);

        measured = now/(HEX_INTERVALS);  
         if(measured < HEX_ALARM_BOUNDARY) {      // ** blink drive  **

            ack_flag = 1;
            uxMessagesWaiting = uxQueueMessagesWaiting(xQblink);
            if(uxMessagesWaiting){  // xQueuePeek does not empty Q
                xQueuePeek(xQblink, &blink_buffer, 0);
            
                if(blink_buffer == 0) show_seven_seg(blanked);
                else show_seven_seg(measured);
                }
            }   // ****  end blink drive  **********
        else {  //  Show values above Alarm  boundary
       
            if(ack_flag != 0) {   // ****  Blink until Ack seen  ***
                if(uxQueueMessagesWaiting(xQblink)){
                    xQueuePeek(xQblink, &blink_buffer, 0);
           
                    if(blink_buffer == 0) show_seven_seg(blanked);
                    else show_seven_seg(measured);
                }
                // ** now check Ack switch to acknowledge blinking **
                if(uxQueueMessagesWaiting(xQsw7)) { // check for ACK, turn ACK flag off
                    xQueueReceive(xQsw7, &sw7_buffer, portMAX_DELAY);
                    sw7_state = sw7_buffer;
                    if (sw7_state == 0) {
                        ack_flag = 0;
                        }
                 }  // end if(uxMessageWaiting)
              
            }   //  end if(ack_flag != 0)
            else show_seven_seg(measured);  // if measure >= ALARM_BOUNDARY
        }   //  End  Show values above Alarm  boundary
        
         vTaskDelay(ms_delay237);  // check adc value every 237 ms
    }  // End while (true)    
}
 

/**
 * @brief Repeatedly flash the Pico's built-in LED.
 *  Time delay of Flash defined by TaskDelayUntil()
 *  Message from SW0 sets blink rate either 600ms or 900 ms. 
 *  xQblink entry remains in Queue buffer until changed so any task can
 *  have blinking LEDs controlled by the blink_task. 
 */
void blink_task(void* unused_arg) {
    
    uint8_t sw0_buffer = 0;
    uint8_t sw1_buffer = 0;
    uint8_t sw_buffer = 0;
    uint8_t swbin_buffer = 0;
    
    int steps = 450;
    uint8_t pico_led_state = 0;
    UBaseType_t uxMessagesWaiting  = 0;
    
   // Initialize start time for vTaskDelayUntil
    TickType_t lastTickTime = xTaskGetTickCount();
        
    while (true) {
        if(uxQueueMessagesWaiting(xQbin2)){  // output of SW 1,2 Binary 0Bxx
            xQueuePeek(xQbin2, &swbin_buffer,0);
        }
        
            switch(swbin_buffer) // four possible blink rates
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
    uint8_t swbin_buffer = 0;
    UBaseType_t uxMessagesWaiting = 0;
    
    while (true) {
        // Check for an item in the blink Queue
        // Read of Queue does not empty it
        uxMessagesWaiting = uxQueueMessagesWaiting(xQblink);
        if(uxQueueMessagesWaiting){
            xQueuePeek(xQblink, &blink_buffer,0);
        }
        // Received a value so flash DOTL and DOTR accordingly
        gpio_put(D14_PIN, blink_buffer == 1 ? 0 : 1);
        gpio_put(D15_PIN, blink_buffer == 0 ? 0 : 1);
        vTaskDelay(ms_delay100);  // check Queue input every 100 ms

   }
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
            if (count < 10) {
                count += 1;
            }
            else {  // reset cout to MIN_COUNT
                count = MIN_COUNT;
             }              //  End if (count < 10)
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
 //           printf(" \n sw_state = %0b", sw_state);
            }

            switch0(sw_state);
            switch1(sw_state);
            switch2(sw_state);
            switch3(sw_state);
            switch4(sw_state);
            switch5(sw_state);
            switch6(sw_state);
            switch7(sw_state);
            switchBin2(sw_state);
            switchBin3(sw_state);
            switchBin4(sw_state); 
            switchBin6(sw_state);
        }   // end else(sw_previous_state |= sw_state)
        vTaskDelay(ms_delay50);  // check switch state every 50 ms
    }  // End while (true)    
}

/*
 * SwitchBin2(int)
 *  Input Switch state value.  The function will Mask out its Switch value 
 * from sw_state and will send Qmessage to task requiring
 * switch change. The switch value is expected to remain in the gueue until
 * changed.  Switches 0,1
 */
void switchBin2(uint8_t sw_state) { //  Switches 0,1 in 8 switch group
    uint8_t now = 0 ;
    
    now = (sw_state & 0B00000011);

    xQueueOverwrite(xQbin2, &now);
}

/*
 * SwitchBin3(int)
 *  Input Switch state value.  The function will Mask out its Switch value 
 * from sw_state and will send Qmessage to task requiring
 * switch change. The switch value is expected to remain in the gueue until
 * changed. Switches 2,3,4
 */
void switchBin3(uint8_t sw_state) { //  Switches 2,3,4 in 8 switch group
    uint8_t now = 0 ;
    
    now = (sw_state & 0B00011100)/4;

    xQueueOverwrite(xQbin3, &now);
}

/*
 * SwitchBin4(int)
 *  Input Switch state value.  The function will Mask out its Switch value 
 * from sw_state and will send Qmessage to task requiring
 * switch change. The switch value is expected to remain in the gueue until
 * changed. Switches 2,3,4
 */
void switchBin4(uint8_t sw_state) { //  Switches 2,3,4 in 8 switch group
    uint8_t now = 0 ;
    
    now = (sw_state & 0B00111100)/4;

    xQueueOverwrite(xQbin4, &now);
}

/*
 * SwitchBin6(int)
 *  Input Switch state value.  The function will Mask out its Switch value 
 * from sw_state and will send Qmessage to task requiring
 * switch change. The switch value is expected to remain in the gueue until
 * changed. Switches 2,3,4
 */
void switchBin6(uint8_t sw_state) { //  Switches 2,3,4 in 8 switch group
    uint8_t now = 0 ;
    
    now = (sw_state & 0B00111111);

    xQueueOverwrite(xQbin6, &now);
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
     }
     else {
         now = 0;
     }
    xQueueOverwrite(xQsw2, &now);
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
    gpio_init(D14_PIN);
    gpio_disable_pulls(D14_PIN);  // remove pullup and pulldowns
    gpio_set_dir(D14_PIN, GPIO_OUT);
    
    // Configure D7_PIN for led_task_gpio 
    gpio_init(D15_PIN);
    gpio_disable_pulls(D15_PIN);  // remove pullup and pulldowns
    gpio_set_dir(D15_PIN, GPIO_OUT);

   
    // Configure ADC
    adc_init();  
    adc_gpio_init(26);   
    adc_select_input(0);

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
    printf("*** Switch Play Program ***");
    printf("\x1B[%i;%iH",4,2);  // place curser
    printf("**************************************\n");
    printf("\x1B[%i;%ir",5,18); // set window top and bottom lines
    printf("\x1B[%i;%iH",5,0);  // place curser

  
    // Set up tasks
    // FROM 1.0.1 Store handles referencing the tasks; get return values
    // NOTE Arg 3 is the stack depth -- in words, not bytes
    BaseType_t blink_status = xTaskCreate(blink_task, 
                                         "BLINK_TASK", 
                                         128, 
                                         NULL, 
                                         8, 
                                         &blink_task_handle);
        if (blink_status != pdPASS) {
            error_state  += 1;
            }
             
    BaseType_t adc_status = xTaskCreate(adc_task,              
                                         "ADC_TASK", 
                                         256, 
                                         NULL, 
                                         7,     // Task priority
                                         &adc_task_handle);
        if (adc_status != pdPASS) {
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

    xQbin2 = xQueueCreate(1, sizeof(uint8_t)); 
    if ( xQbin2 == NULL ) error_state += 1;

    xQbin3 = xQueueCreate(1, sizeof(uint8_t)); 
    if ( xQbin3 == NULL ) error_state += 1;

    xQbin4 = xQueueCreate(1, sizeof(uint8_t)); 
    if ( xQbin4 == NULL ) error_state += 1;

    xQbin6 = xQueueCreate(1, sizeof(uint8_t)); 
    if ( xQbin6 == NULL ) error_state += 1;

    xQadc = xQueueCreate(1, sizeof(uint16_t)); 
    if ( xQadc == NULL ) error_state += 1;
    
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
