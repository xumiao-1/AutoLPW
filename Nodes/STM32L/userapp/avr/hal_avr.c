
/*============================ INCLUDE =======================================*/
#include <stdlib.h>

#include "stm32l1xx_rcc.h"
#include "stm32l1xx_gpio.h"
#include "stm32l1xx_spi.h"
#include "misc.h"
#include "stm32l1xx_syscfg.h"
#include "misc.h"


#include "rf_type.h"

#if(RF_PLATFORM==ZGB_LINK_230)
#include "at86rf230.h"
#elif(RF_PLATFORM==ZGB_LINK_212)
#include "at86rf212.h"
#elif(RF_PLATFORM==ZGB_LINK_231)
#include "at86rf231.h"
#endif
#include "compiler.h"
#include "hal_avr.h"
#include "hal.h"
#include"com.h"
#include "function.h"
#include "user_function.h"
#include "stm32_timer.h"
#include "compiler_avr.h"

#include "stm32l1xx.h"
#include "stm32l1xx_flash.h"
//#include "return.h"


/*============================ MACROS ========================================*/

/*
 * Macros defined for the radio transceiver's access modes.
 *
 * These functions are implemented as macros since they are used very often and
 * we want to remove the function call overhead.
 */
#define HAL_DUMMY_READ         ( 0x00 ) //!< Dummy value for the SPI.

#define HAL_TRX_CMD_RW         ( 0xC0 ) //!< Register Write (short mode).
#define HAL_TRX_CMD_RR         ( 0x80 ) //!< Register Read (short mode).
#define HAL_TRX_CMD_FW         ( 0x60 ) //!< Frame Transmit Mode (long mode).
#define HAL_TRX_CMD_FR         ( 0x20 ) //!< Frame Receive Mode (long mode).
#define HAL_TRX_CMD_SW         ( 0x40 ) //!< SRAM Write.
#define HAL_TRX_CMD_SR         ( 0x00 ) //!< SRAM Read.
#define HAL_TRX_CMD_RADDRM     ( 0x7F ) //!< Register Address Mask.
#define SPI_DUMMY_VALUE                 (0x00)
#define HAL_CALCULATED_CRC_OK   ( 0 ) //!< CRC calculated over the frame including the CRC field should be 0.
/*============================ TYPDEFS =======================================*/
/*============================ VARIABLES =====================================*/
/*! \brief This is a file internal variable that contains the 16 MSB of the 
 *         system time.
 *
 *         The system time (32-bit) is the current time in microseconds. For the
 *         AVR microcontroller implementation this is solved by using a 16-bit 
 *         timer (Timer1) with a clock frequency of 1MHz. The hal_system_time is
 *         incremented when the 16-bit timer overflows, representing the 16 MSB.
 *         The timer value it self (TCNT1) is then the 16 LSB.
 *
 *  \see hal_get_system_time
 */
//static uint16_t hal_system_time = 0;

/*Flag section.*/
static uint8_t volatile hal_bat_low_flag; //!< BAT_LOW flag.
static uint8_t volatile hal_trx_ur_flag; //!< TRX_UR flag.
static uint8_t volatile hal_trx_end_flag; //!< TRX_END flag.
static uint8_t volatile hal_rx_start_flag; //!< RX_START falg;  
static uint8_t volatile hal_unknown_isr_flag; //!< Error, unknown interrupt event signaled from the radio transceiver.
static uint8_t volatile hal_pll_unlock_flag; //!< PLL_UNLOCK flag.
static uint8_t volatile hal_pll_lock_flag;   //!< PLL_LOCK flag.

static uint8_t volatile temp=0;
static bool  volatile time2_over;
//static uint8_t Timer0_value;
//static bool hal_timer0_flag;
/*Callbacks.*/

/*! \brief This function is called when a rx_start interrupt is signaled.
 *
 *         If this function pointer is set to something else than NULL, it will
 *         be called when a RX_START event is signaled. The function takes two
 *         parameters: timestamp in IEEE 802.15.4 symbols (16 us resolution) and 
 *         frame length. The event handler will be called in the interrupt domain, 
 *         so the function must be kept short and not be blocking! Otherwise the 
 *         system performance will be greatly degraded.
 *
 *  \see hal_set_rx_start_event_handler
 */
static hal_rx_start_isr_event_handler_t rx_start_callback;

/*! \brief This function is called when a trx_end interrupt is signaled.
 *
 *         If this function pointer is set to something else than NULL, it will
 *         be called when a TRX_END event is signaled. The function takes two
 *         parameters: timestamp in IEEE 802.15.4 symbols (16 us resolution) and 
 *         frame length. The event handler will be called in the interrupt domain, 
 *         so the function must be kept short and not be blocking! Otherwise the 
 *         interrupt performance will be greatly degraded.
 *
 *  \see hal_set_trx_end_event_handler
 */
static hal_trx_end_isr_event_handler_t trx_end_callback;
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/
/*bool hal_get_timer0_flag(void)
{
	return (hal_timer0_flag);
}

void  hal_clear_timer0_flag(void )
{
	hal_timer0_flag = false;
}
void hal_set_timer0_flag( void )
{
	hal_timer0_flag = true;
}
void Timer0_init(uint8_t TCN_vlaue)
{
	
}

void  hal_time2_init(void)
{
     	    	
}
bool hal_get_time2_over_flag(void)
{
	//return time2_over;
	return 1;
}

void hal_clear_timer2_over_flag(void)
{
	//time2_over=false;
}*/
//*************************************************************************//
void hal_en_net_led(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 

	GPIO_InitStructure.GPIO_Pin = HAL_PIN_NET_LED;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;//GPIO_Speed_40MHz;//Miao
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(HAL_PORT_NET_LED, &GPIO_InitStructure);

	hal_clear_net_led();
}

void hal_en_data_led(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 

	GPIO_InitStructure.GPIO_Pin = HAL_PIN_DATA_LED;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;//GPIO_Speed_40MHz;//Miao
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(HAL_PORT_DATA_LED, &GPIO_InitStructure);

	hal_clear_data_led();
}


//*************************************************************************//
//void hal_mco_output_enable(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA , ENABLE);
//	// Output HSE clock on MCO pin 
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	RCC_MCOConfig(RCC_MCOSource_HSE,RCC_MCODiv_1);//RCC_MCOSource_HSE
//}

void hal_flash_power_config(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;//enable power switch
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_8);
}



//**************************************************************************//
void hal_system_RCC_init(void)
{
	
	RCC_DeInit();//RCC system reset(for debug purpose) 
	RCC_HSEConfig(RCC_HSE_ON);//Enable HSE 
	//RCC_HSEConfig(RCC_HSE_ON);

	if ( RCC_WaitForHSEStartUp() == SUCCESS)
	{
		FLASH_ReadAccess64Cmd(ENABLE);
		FLASH_PrefetchBufferCmd(ENABLE);

		// Flash 0 wait state 
		FLASH_SetLatency(FLASH_Latency_1);

		//Power enable 
		RCC->APB1ENR |= RCC_APB1ENR_PWREN;

		// Select the Voltage Range 1 (1.8 V) */
		PWR->CR = PWR_CR_VOS_0;
		
		// HCLK = SYSCLK 
		RCC_HCLKConfig(RCC_SYSCLK_Div1); 
		// PCLK2 = HCLK 
		RCC_PCLK2Config(RCC_HCLK_Div1); 
		// PCLK1 = HCLK 
    	RCC_PCLK1Config(RCC_HCLK_Div1);
		 
		//PLLCLK = 16MHz *4/2 = 32 MHz 
		RCC_PLLConfig(RCC_PLLSource_HSE, RCC_PLLMul_4,RCC_PLLDiv_2);//RCC_PLLDiv_2
		//RCC_PLLConfig(RCC_PLLSource_HSI, RCC_PLLMul_4,RCC_PLLDiv_2);
		
		RCC_PLLCmd(ENABLE);// Enable PLL
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)// Wait till PLL is ready 
		{}
		
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);//Select PLL as system clock source

		//RCC_HCLKConfig(RCC_SYSCLK_Div1);// HCLK = SYSCLK
		//RCC_PCLK2Config(RCC_HCLK_Div1);// PCLK2 = HCLK/4 
		//RCC_PCLK1Config(RCC_HCLK_Div1);//RCC_HCLK_Div1    PCLK1 = HCLK/1

		  /* Wait till PLL is used as system clock source */
	   // while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_HSE)
	   // {
	   // }

	}
	//SystemInit();

	/*RCC_ClocksTypeDef RCC_ClockFreq;
	
	RCC_ClockFreq.SYSCLK_Frequency=RCC_SYSCLKSource_HSE;
	RCC_ClockFreq.HCLK_Frequency=RCC_SYSCLK_Div1;
	RCC_ClockFreq.PCLK1_Frequency=RCC_HCLK_Div1;
	RCC_ClockFreq.PCLK2_Frequency=RCC_HCLK_Div2;

	RCC_GetClocksFreq(&RCC_ClockFreq);*/
	RCC_ClockSecuritySystemCmd(ENABLE);
	
}

//*************************************************************************//
void hal_SPI_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	// Enable SCK, MOSI and MISO GPIO clocks 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG , ENABLE);
	// GPIOA, GPIOB and SPI1 clock enable 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 , ENABLE);

	GPIO_PinAFConfig(HAL_PORT_SPI, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(HAL_PORT_SPI, GPIO_PinSource7, GPIO_AF_SPI1);
	GPIO_PinAFConfig(HAL_PORT_SPI, GPIO_PinSource6, GPIO_AF_SPI1);
	//GPIO_PinAFConfig(HAL_PORT_SPI, GPIO_PinSource4, GPIO_AF_SPI1);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;

	// SPI SCK pin configuration 
	GPIO_InitStructure.GPIO_Pin = HAL_SCK_PIN;
	GPIO_Init(HAL_PORT_SPI, &GPIO_InitStructure);
	// SPI  MOSI pin configuration 
	GPIO_InitStructure.GPIO_Pin =  HAL_MOSI_PIN;
	GPIO_Init(HAL_PORT_SPI, &GPIO_InitStructure);
	// SPI MISO pin configuration 
	GPIO_InitStructure.GPIO_Pin = HAL_MISO_PIN;
	GPIO_Init(HAL_PORT_SPI, &GPIO_InitStructure);
	  // SPI NSS pin configuration 
	//GPIO_InitStructure.GPIO_Pin = HAL_SS_PIN;
	//GPIO_Init(HAL_PORT_SPI, &GPIO_InitStructure);

	// RST-PB5 SLP-PB6
	GPIO_InitStructure.GPIO_Pin = HAL_RF_RST | HAL_SLP_TR;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(HAL_PORT_SLP_TR, HAL_SLP_TR);
	GPIO_SetBits(HAL_PORT_RST, HAL_RF_RST);
	
	// SS-PA4
	GPIO_InitStructure.GPIO_Pin = HAL_SS_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA, HAL_SS_PIN);
	//GPIO_SetBits(GPIOB, GPIO_Pin_5);

	/* SPI1 configuration ------------------------------------------------------*/
	SPI_I2S_DeInit(SPI1);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
	/* Enable SPI1 */
	SPI_Cmd(SPI1, ENABLE);
}
//**************************************************************************//
#ifdef V13
void hal_Input_Capture_init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB , ENABLE);
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//V1.1: PB4...;   V1.2: PB0
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	//GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4);
	//NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource4);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line4;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	//it_isr_init(ISR_INCAP, hal_input_capture_isr);
}
#else // V12
void hal_Input_Capture_init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB , ENABLE);
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//V1.1: PB4...;   V1.2: PB0
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	//GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource0);

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

	//it_isr_init(ISR_INCAP, hal_input_capture_isr);
}
#endif


//*************************************************************************//
void   hal_init( uint8_t time0_vlaue )
{
    
	/*Reset variables used in file.*/
//	hal_system_time = 0;
	hal_reset_flags( );
//	Timer0_value=time0_vlaue;
	
	hal_en_net_led();
	hal_en_data_led();


	//SPI Specific Initialization.    
	hal_SPI_init();

	hal_Input_Capture_init();

}


uint8_t hal_RF_SPI_Send_Data(uint8_t send_data)
{		 
	//while(1)
	//{
	while((SPI1->SR &SPI_I2S_FLAG_TXE)==RESET);
	SPI1->DR = send_data;
	
	//}
	while((SPI1->SR &SPI_I2S_FLAG_RXNE)==RESET);
	return(SPI1->DR);
	// Loop while DR register in not emplty 
	/*while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);	
	// Send byte through the SPI1 peripheral 
	SPI_I2S_SendData(SPI1, send_data);
	// Wait to receive a byte 
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) ==RESET);
	// Return the byte read from the SPI bus 
	return SPI_I2S_ReceiveData(SPI1);*/
}

/*! \brief  This function reset the interrupt flags and interrupt event handlers 
 *          (Callbacks) to their default value.
 *
 *  \ingroup hal_avr_api
 */
void hal_reset_flags( void ){
    
    AVR_ENTER_CRITICAL_REGION( );
    
    //Reset Flags.
    hal_bat_low_flag     = 0;
    hal_trx_ur_flag      = 0;
    hal_trx_end_flag     = 0;
    hal_rx_start_flag    = 0;  
    hal_unknown_isr_flag = 0;
    hal_pll_unlock_flag  = 0;
    hal_pll_lock_flag    = 0;
    
    //Reset Associated Event Handlers.
    rx_start_callback = NULL;
    trx_end_callback  = NULL;
    
    AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief  This function returns the current value of the BAT_LOW flag.
 *
 *  The BAT_LOW flag is incremented each time a BAT_LOW event is signaled from the
 *  radio transceiver. This way it is possible for the end user to poll the flag 
 *  for new event occurances. 
 *
 *  \ingroup hal_avr_api
 */
uint8_t hal_get_bat_low_flag( void ){
    return hal_bat_low_flag;
}

/*! \brief  This function clears the BAT_LOW flag.
 *
 *  \ingroup hal_avr_api
 */
void hal_clear_bat_low_flag( void ){
    
    AVR_ENTER_CRITICAL_REGION( );
    hal_bat_low_flag = 0;
    AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief  This function returns the current value of the TRX_UR flag.
 *
 *  The TRX_UR flag is incremented each time a TRX_UR event is signaled from the
 *  radio transceiver. This way it is possible for the end user to poll the flag 
 *  for new event occurances. 
 *
 *  \ingroup hal_avr_api
 */
uint8_t hal_get_trx_ur_flag( void ){
    return hal_trx_ur_flag;
}

/*! \brief  This function clears the TRX_UR flag.
 *
 *  \ingroup hal_avr_api
 */
void hal_clear_trx_ur_flag( void ){
    
    AVR_ENTER_CRITICAL_REGION( );
    hal_trx_ur_flag = 0;
    AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief  This function returns the current value of the TRX_END flag.
 *
 *  The TRX_END flag is incremented each time a TRX_END event is signaled from the
 *  radio transceiver. This way it is possible for the end user to poll the flag 
 *  for new event occurances. 
 *
 *  \ingroup hal_avr_api
 */
uint8_t hal_get_trx_end_flag( void ){
    return hal_trx_end_flag;
}

/*! \brief  This function clears the TRX_END flag.
 *
 *  \ingroup hal_avr_api
 */
void hal_clear_trx_end_flag( void ){
    
    AVR_ENTER_CRITICAL_REGION( );
    hal_trx_end_flag = 0;
    AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief  This function is used to set new TRX_END event handler, overriding 
 *          old handler reference.
 *
 *  \ingroup hal_avr_api
 */
hal_trx_end_isr_event_handler_t hal_get_trx_end_event_handler( void ){
    return trx_end_callback;
}

/*! \brief  This function is used to set new TRX_END event handler, overriding 
 *          old handler reference.
 *
 *  \ingroup hal_avr_api
 */
void hal_set_trx_end_event_handler( hal_trx_end_isr_event_handler_t trx_end_callback_handle ){
    
    AVR_ENTER_CRITICAL_REGION( );
    trx_end_callback = trx_end_callback_handle;
    AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief  Remove event handler reference.
 *
 *  \ingroup hal_avr_api
 */
void hal_clear_trx_end_event_handler( void ){
    
    AVR_ENTER_CRITICAL_REGION( );
    trx_end_callback = NULL;
    AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief  This function returns the current value of the RX_START flag.
 *
 *  The RX_START flag is incremented each time a RX_START event is signaled from the
 *  radio transceiver. This way it is possible for the end user to poll the flag 
 *  for new event occurances. 
 *
 *  \ingroup hal_avr_api
 */
uint8_t hal_get_rx_start_flag( void ){
    return hal_rx_start_flag;
}

/*! \brief  This function clears the RX_START flag.
 *
 *  \ingroup hal_avr_api
 */
void hal_clear_rx_start_flag( void ){
    
    AVR_ENTER_CRITICAL_REGION( );
    hal_rx_start_flag = 0;
    AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief  This function returns the active RX_START event handler
 *
 *  \return Current RX_START event handler registered.
 *  \ingroup hal_avr_api
 */
hal_rx_start_isr_event_handler_t hal_get_rx_start_event_handler( void ){
    return rx_start_callback;    
}

/*! \brief  This function is used to set new RX_START event handler, overriding 
 *          old handler reference.
 *
 *  \ingroup hal_avr_api
 */
void hal_set_rx_start_event_handler( hal_rx_start_isr_event_handler_t rx_start_callback_handle ){
    
    AVR_ENTER_CRITICAL_REGION( );
    rx_start_callback = rx_start_callback_handle;
    AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief  Remove event handler reference.
 *
 *  \ingroup hal_avr_api
 */
void hal_clear_rx_start_event_handler( void ){
    
    AVR_ENTER_CRITICAL_REGION( );
    rx_start_callback = NULL;
    AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief  This function returns the current value of the UNKNOWN_ISR flag.
 *
 *  The UNKNOWN_ISR flag is incremented each time an UNKNOWN_ISR event is signaled from the
 *  radio transceiver. This way it is possible for the end user to poll the flag 
 *  for new event occurances. 
 *
 *  \ingroup hal_avr_api
 */
uint8_t hal_get_unknown_isr_flag( void ){
    return hal_unknown_isr_flag;
}

/*! \brief  This function clears the UNKNOWN_ISR flag.
 *
 *  \ingroup hal_avr_api
 */
void hal_clear_unknown_isr_flag( void ){
    
    AVR_ENTER_CRITICAL_REGION( );
    hal_unknown_isr_flag = 0;
    AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief  This function returns the current value of the PLL_UNLOCK flag.
 *
 *  The PLL_UNLOCK flag is incremented each time a PLL_UNLOCK event is signaled from the
 *  radio transceiver. This way it is possible for the end user to poll the flag 
 *  for new event occurances. 
 *
 *  \ingroup hal_avr_api
 */
uint8_t hal_get_pll_unlock_flag( void ){
    return hal_pll_unlock_flag;
}

/*! \brief  This function clears the PLL_UNLOCK flag.
 *
 *  \ingroup hal_avr_api
 */
void hal_clear_pll_unlock_flag( void ){
    
    AVR_ENTER_CRITICAL_REGION( );
    hal_pll_unlock_flag = 0;
    AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief  This function returns the current value of the PLL_LOCK flag.
 *
 *  The PLL_LOCK flag is incremented each time a PLL_LOCK event is signaled from the
 *  radio transceiver. This way it is possible for the end user to poll the flag 
 *  for new event occurances. 
 *
 *  \ingroup hal_avr_api
 */
uint8_t hal_get_pll_lock_flag( void ){
    return hal_pll_lock_flag;
}

/*! \brief  This function clears the PLL_LOCK flag.
 *
 *  \ingroup hal_avr_api
 */
void hal_clear_pll_lock_flag( void ){
    
    AVR_ENTER_CRITICAL_REGION( );
    hal_pll_lock_flag = 0;
    AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief  This function reads data from one of the radio transceiver's registers.
 *
 *  \param  address Register address to read from. See datasheet for register 
 *                  map.
 *
 *  \see Look at the at86rf230_registermap.h file for register address definitions.
 *
 *  \returns The actual value of the read register.
 *
 *  \ingroup hal_avr_api
 */
uint8_t hal_register_read( uint8_t address ){
     uint8_t register_value = 0;
	//Add the register read command to the register address.
	address &= HAL_TRX_CMD_RADDRM;
	address |= HAL_TRX_CMD_RR;

	AVR_ENTER_CRITICAL_REGION( );

	HAL_SS_LOW( ); //Start the SPI transaction by pulling the Slave Select low.

	/*Send Register address and read register content.*/

	register_value=hal_RF_SPI_Send_Data(address);
	register_value=hal_RF_SPI_Send_Data(register_value);		

	HAL_SS_HIGH( ); //End the transaction by pulling the Slave Select High.  

	AVR_LEAVE_CRITICAL_REGION( );

	return register_value;
}

/*! \brief  This function writes a new value to one of the radio transceiver's 
 *          registers.
 *
 *  \see Look at the at86rf230_registermap.h file for register address definitions.
 *
 *  \param  address Address of register to write.
 *  \param  value   Value to write.
 *
 *  \ingroup hal_avr_api
 */
void hal_register_write( uint8_t address, uint8_t value )
{
	// uint8_t dummy_read;                                    
	//Add the Register Write command to the address.
	address = HAL_TRX_CMD_RW | (HAL_TRX_CMD_RADDRM & address);

	AVR_ENTER_CRITICAL_REGION( );    

	HAL_SS_LOW( ); //Start the SPI transaction by pulling the Slave Select low.

	/*Send Register address and write register content.*/

	hal_RF_SPI_Send_Data(address);	
	delay_us(100);
	
	hal_RF_SPI_Send_Data( value);		

	HAL_SS_HIGH( ); //End the transaction by pulling the Slave Slect High.

	AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief  This function reads the value of a specific subregister.
 *
 *  \see Look at the at86rf230_registermap.h file for register and subregister 
 *       definitions.
 *
 *  \param  address  Main register's address.
 *  \param  mask  Bit mask of the subregister.
 *  \param  position   Bit position of the subregister
 *  \retval Value of the read subregister.
 *
 *  \ingroup hal_avr_api
 */
uint8_t hal_subregister_read( uint8_t address, uint8_t mask, uint8_t position ){
    
    //Read current register value and mask out subregister.
    uint8_t register_value = hal_register_read( address );
    register_value &= mask;
    register_value >>= position; //Align subregister value.
    
    return register_value;
}

/*! \brief  This function writes a new value to one of the radio transceiver's 
 *          subregisters.
 *
 *  \see Look at the at86rf230_registermap.h file for register and subregister 
 *       definitions.
 *
 *  \param  address  Main register's address.
 *  \param  mask  Bit mask of the subregister.
 *  \param  position  Bit position of the subregister
 *  \param  value  Value to write into the subregister.
 *
 *  \ingroup hal_avr_api
 */
void hal_subregister_write( uint8_t address, uint8_t mask, uint8_t position, 
                            uint8_t value ){
    
    //Read current register value and mask area outside the subregister.
    uint8_t register_value = hal_register_read( address );
    register_value &= ~mask;
    
    //Start preparing the new subregister value. shift in place and mask.
    value <<= position;
    value &= mask;
    
    value |= register_value; //Set the new subregister value.

    //Write the modified register value.
    hal_register_write( address, value );
}

/*! \brief  This function will upload a frame from the radio transceiver's frame 
 *          buffer.
 *          
 *          If the frame currently available in the radio transceiver's frame buffer
 *          is out of the defined bounds. Then the frame length, lqi value and crc
 *          be set to zero. This is done to indicate an error.
 *
 *  \param  rx_frame    Pointer to the data structure where the frame is stored.
 *
 *  \ingroup hal_avr_api
 */
 void hal_frame_read( hal_rx_frame_t *rx_frame )
{
    uint8_t frame_length;
    AVR_ENTER_CRITICAL_REGION( );
    
    HAL_SS_LOW( );
    
    //Send frame read command.

    frame_length=hal_RF_SPI_Send_Data(HAL_TRX_CMD_FR);	//
    frame_length=hal_RF_SPI_Send_Data(frame_length);

   //delay_ms(1);

   
    /*Check for correct frame length.*/
    if ((frame_length >= HAL_MIN_FRAME_LENGTH) && (frame_length <= HAL_MAX_FRAME_LENGTH)) {
		uint16_t crc = 0;
		uint8_t *rx_data = (rx_frame->data);
		uint8_t tempData;

		rx_frame->length = frame_length; //Store frame length.

		/*Upload frame buffer to data pointer. Calculate CRC.*/
		do{
			tempData = hal_RF_SPI_Send_Data(frame_length);
			*rx_data++ = tempData;      
			crc = crc_ccitt_update( crc, tempData );
		} while (--frame_length > 0);
        
        /*Read LQI value for this frame.*/
        rx_frame->lqi = hal_RF_SPI_Send_Data( tempData);

        HAL_SS_HIGH( );
        
        /*Check calculated crc, and set crc field in hal_rx_frame_t accordingly.*/
        if (crc == HAL_CALCULATED_CRC_OK) {
            rx_frame->crc = true; 
        } else { rx_frame->crc = false; }
    } else {
        
        HAL_SS_HIGH( );
        
        rx_frame->length = 0;
        rx_frame->lqi    = 0;
        rx_frame->crc    = false;    
    }
    
    AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief  This function will download a frame to the radio transceiver's frame 
 *          buffer.
 *
 *  \param  write_buffer    Pointer to data that is to be written to frame buffer.
 *  \param  length          Length of data. The maximum length is 127 bytes.
 *
 *  \ingroup hal_avr_api
 */
 void hal_frame_write( uint8_t *write_buffer, uint8_t length )
{
    uint8_t dummy_read;
	
    length &= HAL_TRX_CMD_RADDRM; //Truncate length to maximum frame length.
    
    AVR_ENTER_CRITICAL_REGION( );
    
    HAL_SS_LOW( ); //Initiate the SPI transaction.
    
    /*SEND FRAME WRITE COMMAND AND FRAME LENGTH.*/
  
    hal_RF_SPI_Send_Data(HAL_TRX_CMD_FW);	
    hal_RF_SPI_Send_Data(length);
   delay_us(100); 
    //Download to the Frame Buffer.
   /* do {
        
        SPDR = *write_buffer++;
        --length;
        
        while ((SPSR & (1 << SPIF)) == 0) {;}
        
        dummy_read = SPDR;
	//delay_us(100);
    } while (length > 0);*/
	do {
		
        dummy_read = hal_RF_SPI_Send_Data( *write_buffer++);
    } while (--length > 0);	
    
    HAL_SS_HIGH( ); //Terminate SPI transaction.
    
    AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief Read SRAM
 *
 * This function reads from the SRAM of the radio transceiver.
 *
 * \param address Address in the TRX's SRAM where the read burst should start
 * \param length Length of the read burst
 * \param data Pointer to buffer where data is stored.
 *
 * \ingroup hal_avr_api
 */
 void hal_sram_read( uint8_t address, uint8_t length, uint8_t *data )
{
//    uint8_t dummy_read;
    AVR_ENTER_CRITICAL_REGION( );
    
    HAL_SS_LOW( ); //Initiate the SPI transaction.
    
    //Send SRAM read command.
    hal_RF_SPI_Send_Data(HAL_TRX_CMD_SR);	
    hal_RF_SPI_Send_Data(address);  	
    /*Upload the chosen memory area.*/
    do {
		
        *data++ = hal_RF_SPI_Send_Data( HAL_DUMMY_READ);
    } while (--length > 0);

    HAL_SS_HIGH( );
    
    AVR_LEAVE_CRITICAL_REGION( );
}

/*! \brief Write SRAM
 *
 * This function writes into the SRAM of the radio transceiver.
 *
 * \param address Address in the TRX's SRAM where the write burst should start
 * \param length  Length of the write burst
 * \param data    Pointer to an array of bytes that should be written
 *
 * \ingroup hal_avr_api
 */
 void hal_sram_write( uint8_t address, uint8_t length, uint8_t *data )
{
   // uint8_t dummy_read;
	
    AVR_ENTER_CRITICAL_REGION( );
        
    HAL_SS_LOW( );
    
    //Send SRAM write command.
   hal_RF_SPI_Send_Data(HAL_TRX_CMD_SW);	
   hal_RF_SPI_Send_Data(address); 
    /*Upload the chosen memory area.*/
    do {
        
        hal_RF_SPI_Send_Data( *data++);
    } while (--length > 0);
    
    HAL_SS_HIGH( );
    
    AVR_LEAVE_CRITICAL_REGION( );
}


/**
 * @brief Writes and reads data into/from SRAM of the transceiver
 *
 * This function writes data into the SRAM of the transceiver and
 * simultaneously reads the bytes.
 *
 * @param addr Start address in the SRAM for the write operation
 * @param idata Pointer to the data written/read into/from SRAM
 * @param length Number of bytes written/read into/from SRAM
 */
 void hal_trx_aes_wrrd(uint8_t addr, uint8_t *idata, uint8_t length)
{
    uint8_t *odata;

    delay_us(1);

    AVR_ENTER_CRITICAL_REGION();
    /* Start SPI transaction by pulling SEL low */
    HAL_SS_LOW();

    // Send the command byte 
    hal_RF_SPI_Send_Data(HAL_TRX_CMD_SW);//Send the command byte 
    hal_RF_SPI_Send_Data(addr);//write SRAM start address

    // now transfer data 
    odata = idata;
  
    /* write data byte 0 - the obtained value in SPDR is meaningless */
    hal_RF_SPI_Send_Data( *idata++);

    /* process data bytes 1...length-1: write and read */
    while (--length)
    {
       *odata++ = hal_RF_SPI_Send_Data( *idata++);
    }

    /* to get the last data byte, write some dummy byte */
    *odata++ = hal_RF_SPI_Send_Data( SPI_DUMMY_VALUE);

    /* Stop the SPI transaction by setting SEL high */
    HAL_SS_HIGH();
    AVR_LEAVE_CRITICAL_REGION();
}


//This #if compile switch is used to provide a "standard" function body for the 
//doxygen documentation.
void hal_input_capture_isr(void)
{
    uint8_t interrupt_source;
    /*The following code reads the current system time. This is done by first 
      reading the hal_system_time and then adding the 16 LSB directly from the
      TCNT1 register.
     */
   // uint32_t isr_timestamp = hal_system_time;
   // isr_timestamp <<= 16;
   // isr_timestamp |= TCNT1; 
    
    /*Read Interrupt source.*/
    HAL_SS_LOW( );
	
  interrupt_source= hal_RF_SPI_Send_Data(RG_IRQ_STATUS | HAL_TRX_CMD_RR);	

  interrupt_source= hal_RF_SPI_Send_Data(interrupt_source);//The interrupt source is read.
 //   SPDR = interrupt_source;
 //   while ((SPSR & (1 << SPIF)) == 0) {;}	
   // interrupt_source = SPDR; //The interrupt source is read.

    HAL_SS_HIGH( );

    /*Handle the incomming interrupt. Prioritized.*/
    if ((interrupt_source & HAL_RX_START_MASK)) {
        
        hal_rx_start_flag++; //Increment RX_START flag.
        
        if( rx_start_callback != NULL ){
             uint8_t frame_length;
            //Read Frame length and call rx_start callback.
            HAL_SS_LOW( );
    
            /*SPDR = HAL_TRX_CMD_FR;
            while ((SPSR & (1 << SPIF)) == 0) {;}
            uint8_t frame_length = SPDR;
        
            SPDR = frame_length; //Any data will do, so frame_length is used.
            while ((SPSR & (1 << SPIF)) == 0) {;}
            frame_length = SPDR;*/

	     frame_length=hal_RF_SPI_Send_Data(HAL_TRX_CMD_FR);	
	     frame_length=hal_RF_SPI_Send_Data(frame_length);	 
            
            HAL_SS_HIGH( );
            
            rx_start_callback( 0, frame_length );
        }
    }
    else if (interrupt_source & HAL_TRX_END_MASK) {
        
        hal_trx_end_flag++; //Increment TRX_END flag.
        
       if( trx_end_callback != NULL ){
            trx_end_callback( 0 );
           // trx_end_handler();
        }
    } 
    else if (interrupt_source & HAL_TRX_UR_MASK) {
        hal_trx_ur_flag++; //Increment TRX_UR flag.    
    } else if (interrupt_source & HAL_PLL_UNLOCK_MASK) {
        hal_pll_unlock_flag++; //Increment PLL_UNLOCK flag.   
    } else if (interrupt_source & HAL_PLL_LOCK_MASK) {
        hal_pll_lock_flag++; //Increment PLL_LOCK flag.
    } else if (interrupt_source & HAL_BAT_LOW_MASK) {

        //Disable BAT_LOW interrupt to prevent interrupt storm. The interrupt 
        //will continously be signaled when the supply voltage is less than the 
        //user defined voltage threshold.
        uint8_t trx_isr_mask = hal_register_read( RG_IRQ_MASK );
        trx_isr_mask &= ~HAL_BAT_LOW_MASK;
        hal_register_write( RG_IRQ_MASK, trx_isr_mask );
        hal_bat_low_flag++; //Increment BAT_LOW flag.
    } else {
        hal_unknown_isr_flag++;  //Increment UNKNOWN_ISR flag.
    } 
}

//This #if compile switch is used to provide a "standard" function body for the 
//doxygen documentation.
/*ISR( TIMER1_OVF_vect ){
    hal_system_time++;
}

ISR(TIMER2_OVF_vect)
{
	TCNT2=0xe0;
	temp++;
	com_send_hex(temp);	
	//com_send_hex(3);
	time2_over=true;
    if(temp>=10)
	{
	TCCR2A=0x00;
	TCCR2B=0x00;
	//TCNT2=0x00;
	TIMSK2=0x00;
	}
}

ISR( TIMER0_OVF_vect )
{
	TIFR0 |= 0x03;
	TCNT0 = Timer0_value;//0x19;
	hal_set_timer0_flag();
}*/
/*EOF*/
