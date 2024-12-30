/**
 * RP2040 FreeRTOS seven_seg.h
 *
 * @copyright 2022, Calvin McCarthy
 * @version   1.0.0
 * @license   MIT
 *
 */
#ifndef _SEVEN_SEG_H_
#define _SEVEN_SEG_H_
#define I2C_ADDR 0x20


/**
 * PROTOTYPES
 */
 
void config_seven_seg();
void show_seven_seg(uint8_t num);
void segment_ON(uint8_t num);
void segment_OFF(uint8_t num);
void blank_seven_seg();
void show_seven_seg_dec(uint8_t num);
void show_seven_seg_hex(uint8_t num);
#ifdef __cplusplus
}           // extern "C"
#endif


#endif      // _MAIN_H_
