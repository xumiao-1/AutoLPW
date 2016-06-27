#include "sensor_gpio.h"
#include "function.h"

#include "stm32l1xx_gpio.h"
#include "misc.h"
#include "stm32l1xx_syscfg.h"
#include "mydef.h"

static volatile uint8_t sensor_gpio_flag = 0;
static EXTI_InitTypeDef EXTI_InitStructure;

void ss_hall_switch_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA , ENABLE);//RCC_APB2Periph_AFIO
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	if (NODEOFF == gConfig.gPhase) { // only enable if NODEOFF
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
	  
		NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	
		EXTI_InitStructure.EXTI_Line = EXTI_Line0;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);
	}
}

//void EXTI0_IRQHandler(void)
//{
//	if(EXTI_GetITStatus(EXTI_Line0) != RESET){
//		EXTI_ClearITPendingBit(EXTI_Line0);
//		sensor_gpio_flag = 1;
//	}
//}

void ss_hall_switch_int_disable(void)
{
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);
}

void ss_hall_switch_int_enable(void)
{
	EXTI_GetITStatus(EXTI_Line0);
	EXTI_ClearITPendingBit(EXTI_Line0);
	
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}

uint8_t ss_get_hall_switch_status(void)
{
	return sensor_gpio_flag;
}

void ss_set_hall_switch_status(uint8_t value)
{
	sensor_gpio_flag = value;
}

uint8_t ss_get_hall_gpio_status(void)
{
	return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
}


