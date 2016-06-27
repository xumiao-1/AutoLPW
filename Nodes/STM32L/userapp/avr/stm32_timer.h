#ifndef STM32_TIMER_H
#define STM32_TIMER_H

//#include "avr_compiler.h"
#include <stdint.h>
#include <stdbool.h>

#define FLAG_TRUE	1
#define FLAG_FALSE	0

typedef enum{
	TIM_INIT_2MS = 0x000E,	// @ 7.3728M/1024
	TIM_INIT_4MS = 0x001C,
	TIM_INIT_5MS = 0x0024,
	TIM_INIT_6MS = 0x002B,
	TIM_INIT_7MS = 0x0032,
	TIM_INIT_10MS = 0x0048,
	TIM_INIT_20MS = 0x0090,
	TIM_INIT_30MS = 0x00D8,
	TIM_INIT_50MS = 0x0168,
	TIM_INIT_80MS = 0x0240,
	TIM_INIT_100MS = 0x02D0,
	TIM_INIT_180MS = 0x0510,
	TIM_INIT_200MS = 0x05A0,
	TIM_INIT_250MS = 0x0708,
	TIM_INIT_300MS = 0x0870,
	TIM_INIT_500MS = 0x0E10,
	TIM_INIT_800MS = 0x1680,
	TIM_INIT_1S = 0x1C20,
	TIM_INIT_2S = 0x3840,
	TIM_INIT_5S = 0x8CA0,
	TIM_INIT_6S = 0xA8C0,
	TIM_INIT_MAX = 0xFFFF	// 9.1s
}ax_tim_initValue_t;

void hal_sysclk_init(void);
void hal_rtc_init(void);
void hal_TimerUsart_init(uint16_t TCN_vlaue);
uint8_t hal_get_TimerUsart_flag(void);
void hal_set_TimerUsart_flag(uint8_t tflag);
uint8_t hal_get_TimerUsart_status(void);
void hal_TimerUsart_disable(void);
void hal_TimerUsart_enable(const uint8_t set_zero);

void ax_timA_init(uint16_t initValue);
void ax_timA_enable(uint16_t initValue);
void ax_timA_continue(void);
void ax_timA_disable(void);
uint8_t ax_timA_get_ov_flag(void);
void ax_timA_set_ov_flag(uint8_t flagValue);
uint8_t ax_timA_get_status(void);

void ax_timB_init(uint16_t initValue);
void ax_timB_enable(uint16_t initValue);
void ax_timB_continue(void);
void ax_timB_disable(void);
uint8_t ax_timB_get_ov_flag(void);
void ax_timB_set_ov_flag(uint8_t flagValue);
uint8_t ax_timB_get_status(void);

//void hal_RTC_OV_isr(void);
uint8_t get_rtc_count(void);
bool get_io_test_flag(void);
void clear_io_test_flag(void);


#endif

