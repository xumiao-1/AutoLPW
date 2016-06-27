/****************************************
 * this file is used for battery monitor
 ****************************************/

#include "bat_monitor.h"

// config pins for battery monitor
void bat_mon_config(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHBPeriphClockCmd(BAT_ADC_PERIPH | BAT_MON_PERIPH, ENABLE);

	// voltage read in --> AN
	GPIO_InitStructure.GPIO_Pin = BAT_ADC_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(BAT_ADC_PORT, &GPIO_InitStructure);

	// voltage output --> OUTPUT, PP, NOPULL
	GPIO_InitStructure.GPIO_Pin = BAT_MON_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(BAT_MON_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(BAT_MON_PORT, BAT_MON_PIN);	
	
	// configure ADC1
	ADC1_Config();
	
}

/**
 *configure ADC2:
 *converts continuously ADC channel10,
 *EOC interrupt is generated.
 */
static void ADC1_Config(void){
	// ADC2 configuration
	ADC_InitTypeDef ADC_InitStructure;
	
	/* Enable The HSI (16Mhz) */
	RCC_HSICmd(ENABLE);
	
	/* Check that HSI oscillator is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);

	// ADC clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	// ADC2 regular channels configuration
	ADC_RegularChannelConfig(ADC1, BAT_ADC_CHANNEL, 1, ADC_SampleTime_192Cycles);

	/* Define delay between ADC1 conversions */
	ADC_DelaySelectionConfig(ADC1, ADC_DelayLength_Freeze);
	
	/* Enable ADC1 Power Down during Delay */
	ADC_PowerDownCmd(ADC1, ADC_PowerDown_Idle_Delay, ENABLE);
  
}

uint16_t measure_battery(void) {
	uint16_t ADCdata;

	// start measurement
	/* Enable The HSI (16Mhz) */
	RCC_HSICmd(ENABLE);
	
	BAT_MON_ON();
	
	/* Check that HSI oscillator is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
	
	// Enable ADC2
	ADC_Cmd(ADC1, ENABLE);
	
	/* Wait until ADC1 ON status */
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET);	
	
	// Start ADC2 Software Conversion 
	ADC_SoftwareStartConv(ADC1);
	
	/* Wait until ADC Channel 5 or 1 end of conversion */
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	
	
	// read adc value
	/* Read ADC conversion result */
	ADCdata = ADC_GetConversionValue(ADC1);
	#ifdef MY_DEBUG
	//com_send_string((uint8_t*)&ADCdata, sizeof(ADCdata));
	#endif
	/* Calculate voltage value*/
	//VDDmV = (uint32_t)((uint32_t)ADCdata * (uint32_t)ADC_CONVERT_RATIO);	
	
	
	// end measurement
	BAT_MON_OFF();
	ADC_Cmd(ADC1, DISABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, DISABLE);

	return ADCdata;
}


// eof...
