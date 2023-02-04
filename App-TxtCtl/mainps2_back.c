/** 
 *  PIO reading PS2 Keyboard
 * PS/2 data input Pin 19, GPIO 14
 * PS/2 clock input Pin 20, GPIO 15
 * output on USB terminal
 * Jan 31, 2023  Calvin McCarthy
 * 
 **/

#include <stdio.h>
#include "pico/stdlib.h"
#include "ps2.h"

//#define UART_ID uart0
//#define BAUD_RATE 115200
//#define UART_TX_PIN 0
//#define UART_RX_PIN 1
#define KBD_PIO pio1

int main() {
    uint8_t inchar;
    uint8_t char_position = 0;
    
    stdio_init_all();

    stdio_usb_init();
    // Pause to allow the USB path to initialize
    sleep_ms(2000);
 
    kbd_init();

    printf("\x1B[2J");  // Clear Screen
    printf("\x1B[%i;%iH",2,3);  // place curser
    puts("*** PS/2 KBD example ***");
    printf("\x1B[%i;%iH",4,3);  // place curser
    puts("**************************************");
    printf("\x1B[%i;%ir",6,20);  // set top and bottom lines of window
    printf("\x1B[%i;%iH",6,0);   // place curser
    
    // dump scan codes
    for (;;) {
	inchar = get_iso8859_code();
	if(inchar == 0x0d) {
	    printf("\n");
	    char_position = 0;
	}
	else if(inchar == 0x7f){
	    if(char_position > 0){
		printf("\x1B[1D");  // move curser back
		printf("%c",0x20);  // erase last char, curser advances
		printf("\x1B[1D");  // move curser back again
		--char_position;
	    }
	    else {
		char_position = 0;
	    }
	}
	else {
	    printf("%c",inchar);
	    char_position++;
	    if(char_position == 128) {  // Lines less than 128 char
		char_position = 0;
		printf("\n");		
	    }
	}
    }
}
