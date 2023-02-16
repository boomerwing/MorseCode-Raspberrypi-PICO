#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "kbdpio.pio.h"
#include "ps2.h"
// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <semphr.h>

#define BREAK 0x01
#define MODIFIER 0x02
#define SHIFT_L 0x04
#define SHIFT_R 0x08
#define ALTGR 0x10

// KBD data and clock inputs must be consecutive with
// data in the lower position.
#define DAT_GPIO 14 // PS/2 data
#define CLK_GPIO 15 // PS/2 clock

#define KBD_PIO pio1
typedef struct
{
	uint8_t noshift[PS2_KEYMAP_SIZE];
	uint8_t shift[PS2_KEYMAP_SIZE];
	unsigned int uses_altgr;
	uint8_t altgr[PS2_KEYMAP_SIZE];
} PS2Keymap_t;

const PS2Keymap_t PS2Keymap_US = {
	// without shift
	{0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
	 0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, '`', 0,
	 0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'q', '1', 0,
	 0, 0, 'z', 's', 'a', 'w', '2', 0,
	 0, 'c', 'x', 'd', 'e', '4', '3', 0,
	 0, ' ', 'v', 'f', 't', 'r', '5', 0,
	 0, 'n', 'b', 'h', 'g', 'y', '6', 0,
	 0, 0, 'm', 'j', 'u', '7', '8', 0,
	 0, ',', 'k', 'i', 'o', '0', '9', 0,
	 0, '.', '/', 'l', ';', 'p', '-', 0,
	 0, 0, '\'', 0, '[', '=', 0, 0,
	 0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, ']', 0, '\\', 0, 0,
	 0, 0, 0, 0, 0, 0, PS2_BACKSPACE, 0,
	 0, '1', 0, '4', '7', 0, 0, 0,
	 '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
	 PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
	 0, 0, 0, PS2_F7},
	// with shift
	{0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
	 0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_SHIFT_TAB, '~', 0,
	 0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'Q', '!', 0,
	 0, 0, 'Z', 'S', 'A', 'W', '@', 0,
	 0, 'C', 'X', 'D', 'E', '$', '#', 0,
	 0, ' ', 'V', 'F', 'T', 'R', '%', 0,
	 0, 'N', 'B', 'H', 'G', 'Y', '^', 0,
	 0, 0, 'M', 'J', 'U', '&', '*', 0,
	 0, '<', 'K', 'I', 'O', ')', '(', 0,
	 0, '>', '?', 'L', ':', 'P', '_', 0,
	 0, 0, '"', 0, '{', '+', 0, 0,
	 0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '}', 0, '|', 0, 0,
	 0, 0, 0, 0, 0, 0, PS2_BACKSPACE, 0,
	 0, '1', 0, '4', '7', 0, 0, 0,
	 '0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
	 PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
	 0, 0, 0, PS2_F7},
	0};

const PS2Keymap_t *keymap = &PS2Keymap_US;

// static uint kbd_sm;
static uint kbd_sm;

char get_iso8859_code(void)
{
	static uint8_t state = 0;
	uint8_t s;
	char c;
	const TickType_t ms_delay50 =   50 / portTICK_PERIOD_MS;

	while (1)
	{
            s = kbd_getc();
		if (!s)
			return 0;
		if (s == 0xF0)
		{
			state |= BREAK;
		}
		else if (s == 0xE0)
		{
			state |= MODIFIER;
		}
		else
		{
			if (state & BREAK)
			{
				if (s == 0x12)
				{
					state &= ~SHIFT_L;
				}
				else if (s == 0x59)
				{
					state &= ~SHIFT_R;
				}
				else if (s == 0x11 && (state & MODIFIER))
				{
					state &= ~ALTGR;
				}
				// CTRL, ALT & WIN keys could be added
				// but is that really worth the overhead?
				state &= ~(BREAK | MODIFIER);
				continue;
			}
			if (s == 0x12)
			{
				state |= SHIFT_L;
				continue;
			}
			else if (s == 0x59)
			{
				state |= SHIFT_R;
				continue;
			}
			else if (s == 0x11 && (state & MODIFIER))
			{
				state |= ALTGR;
			}
			c = 0;
			if (state & MODIFIER)
			{
				switch (s)
				{
				case 0x70:
					c = PS2_INSERT;
					break;
				case 0x6C:
					c = PS2_HOME;
					break;
				case 0x7D:
					c = PS2_PAGEUP;
					break;
				case 0x71:
					c = PS2_DELETE;
					break;
				case 0x69:
					c = PS2_END;
					break;
				case 0x7A:
					c = PS2_PAGEDOWN;
					break;
				case 0x75:
					c = PS2_UPARROW;
					break;
				case 0x6B:
					c = PS2_LEFTARROW;
					break;
				case 0x72:
					c = PS2_DOWNARROW;
					break;
				case 0x74:
					c = PS2_RIGHTARROW;
					break;
				case 0x4A:
					c = '/';
					break;
				case 0x5A:
					c = PS2_ENTER;
					break;
				default:
					break;
				}
			}
			else if ((state & ALTGR) && keymap->uses_altgr)
			{
				if (s < PS2_KEYMAP_SIZE)
					c = keymap->altgr[s]; // pgm_read_byte(keymap->altgr + s);
			}
			else if (state & (SHIFT_L | SHIFT_R))
			{
				if (s < PS2_KEYMAP_SIZE)
					c = keymap->shift[s]; // pgm_read_byte(keymap->shift + s);
			}
			else
			{
				if (s < PS2_KEYMAP_SIZE)
					c = keymap->noshift[s]; // pgm_read_byte(keymap->noshift + s);
			}
			state &= ~(BREAK | MODIFIER);
			if (c)
				return c;
		}
	}
}

static uint8_t kbd_getc(void) {
	const TickType_t ms_delay50 =   50 / portTICK_PERIOD_MS;
    // pull a scan code from the PIO SM fifo
    while(pio_sm_is_rx_fifo_empty(KBD_PIO, kbd_sm)){
//        tight_loop_contents();
        vTaskDelay(ms_delay50);
    }        
    // strip the start, parity, and stop bits
    return KBD_PIO->rxf[kbd_sm] >> 22;
}

/*
void PS2_selectKeyMap(PS2Keymap_t *km)
{
	keymap = km;
}
*/


void kbd_init(void) {

    // init KBD pins to input
    gpio_init(DAT_GPIO);
    gpio_init(CLK_GPIO);
    
    // with pull up 
    gpio_pull_up(DAT_GPIO);
    gpio_pull_up(CLK_GPIO);
    
    // get a state machine
    kbd_sm = pio_claim_unused_sm(KBD_PIO, true);
    
    // copy program to SM memory
    uint offset = pio_add_program(KBD_PIO, &ps2kbd_program);
    
    
    // Set pin directions via pio call (not sure this is required)
    pio_sm_set_consecutive_pindirs(KBD_PIO, kbd_sm, DAT_GPIO, 2, false);
    
    // program the start and wrap SM registers
    pio_sm_config c = ps2kbd_program_get_default_config(offset);
    
    // Set the base input pin. pin index 0 is DAT, index 1 is CLK
    sm_config_set_in_pins(&c, DAT_GPIO);
    
    // Shift 11 bits to the right, autopush enabled
    sm_config_set_in_shift(&c, true, true, 11);
    
    // Deeper FIFO as we're not doing any TX
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    
    // We don't expect clock faster than 16KHz
    float div = (float)clock_get_hz(clk_sys) / (8 * 16000);
    sm_config_set_clkdiv(&c, div);
    
    // Ready to go
    pio_sm_init(KBD_PIO, kbd_sm, offset, &c);
    pio_sm_set_enabled(KBD_PIO, kbd_sm, true);
}

