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

//                  A L            R CE   GBFD
//   0b0000|0000|000x|xx00|0000|0000|xx00|xx00|

#define			SA_PIN	 20
#define			SB_PIN		2
#define			SC_PIN		7
#define			SD_PIN		0
#define			SE_PIN		6
#define			SF_PIN		1
#define			SG_PIN		3
#define			DOTL 		  19  
#define			DOTR 		  8  
#define			D4_PIN		6
#define			D5_PIN		7
#define			D6_PIN		8
#define			D24_PIN		24
#define			D25_PIN		25
#define			TX_PIN		0
#define			RX_PIN		1
#define			SDA_PIN		2
#define			SCL_PIN		3
#define			SCK_PIN		18
#define			MO_PIN		19
#define			MI_PIN		20
#define			GPIO0		0
#define			GPIO1		1
#define			GPIO2		2
#define			GPIO3		3
#define			GPIO6		6
#define			GPIO7		7
#define			GPIO8		8
#define			GPIO9		9
#define			GPIO10		10
#define			GPIO11		11
#define			GPIO12		12
#define			GPIO13		13
#define			GPIO18		18
#define			GPIO19		19
#define			GPIO20		20
#define			GPIO24  	24
#define			GPIO25  	25
  


/**
 * PROTOTYPES
 */
void config_seven_seg();
void show_seven_seg_dec(uint8_t num);
void show_seven_seg_hex(uint8_t num);
void show_seven_seg(uint8_t num);
void led_0();
void led_1();
void led_2();
void led_3();
void led_4();
void led_5();
void led_6();
void led_7();
void led_8();
void led_9();
void led_A();
void led_B();
void led_C();
void led_D();
void led_E();
void led_F();

#ifdef __cplusplus
}           // extern "C"
#endif


#endif      // _MAIN_H_
