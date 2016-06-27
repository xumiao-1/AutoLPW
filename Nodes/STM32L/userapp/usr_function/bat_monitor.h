#ifndef _BAT_MONITOR_H_
#define _BAT_MONITOR_H_


#include "include.h"

//============ MACRO ===================
#define BAT_ADC_PORT (GPIOA)
#define BAT_ADC_PIN (GPIO_Pin_1)
#define BAT_ADC_PERIPH (RCC_AHBPeriph_GPIOA)

#define BAT_MON_PORT (GPIOA)
#define BAT_MON_PIN (GPIO_Pin_2)
#define BAT_MON_PERIPH (RCC_AHBPeriph_GPIOA)

#define BAT_ADC_CHANNEL (ADC_Channel_1)

#define BAT_MON_ON()	(BAT_MON_PORT->BSRRL = BAT_MON_PIN)
#define BAT_MON_OFF()	(BAT_MON_PORT->BSRRH = BAT_MON_PIN)

//============ functions ===============
// config pins for battery monitor
void bat_mon_config(void);
static void ADC1_Config(void);
uint16_t measure_battery(void);


#endif //_BAT_MONITOR_H_

// eof...
