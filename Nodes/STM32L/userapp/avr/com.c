/*============================ INCLDUE =======================================*/
#include <stdint.h>
#include <stdbool.h>

#include "config_uart_extended.h"
#include "stm32l1xx_gpio.h"
#include "stm32l1xx_usart.h"
#include "misc.h"
#include "stm32l1xx_syscfg.h"
#include "stm32l1xx_tim.h"


#include "compiler.h"
#include "com.h"
#include "hal.h"
#include "stm32_timer.h"

/*============================ MACROS ========================================*/
#define COM_RX_MAX_BYTES (COM_RX_BUFFER_SIZE - 2) //!< Maximal number of bytes that one message can contain (PSDU_LENGTH - CRC_LENGTH).
/*============================ TYPEDEFS ======================================*/
/*============================ VARIABLES =====================================*/
static uint8_t com_buffer[ COM_RX_BUFFER_SIZE ]; //!< Array storing rx data.
static uint8_t com_number_of_received_bytes; //!< Number of bytes in com_buffer.
static bool com_data_reception_finished; //!< Flag indicating EOT (That "\r\n is received.")
static uint8_t hex_lookup[ ] = "0123456789ABCDEF"; //!< Look up table for hexadecimal number conversion.
/*============================ PROTOTYPES ====================================*/
//static bool HT_flag;
//static uint8_t Timer0_Initvalue;
/*! \brief This function initializes the chosen communication interface (USB or USART).
 *  
 *  \param[in] rate Baudrate used by the AVR's USART.
 */

 bool Get_com_Rx_Finished_flag(void)
{
	return ( com_data_reception_finished );
}
void  Clear_com_Rx_Finished_flag(void )
{
	com_data_reception_finished = false;
}
void Set_com_Rx_Finished_flag( void )
{
	com_data_reception_finished = true;
}

void com_init( uint32_t band_rate)
{
    
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	/* NVIC configuration */
	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Enable GPIO clock
	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOA , ENABLE);
	// Enable USART clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	// Connect PXx to USARTx_Tx 
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);

	// Connect PXx to USARTx_Rx 
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	// Configure USART Tx and Rx as alternate function push-pull 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;//GPIO_Speed_40MHz;//Miao
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = band_rate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	// Enable USART 
	USART_Cmd(USART1, ENABLE);
	hal_TimerUsart_init(0x00D8);

	com_reset_receiver();

}

//*************************************************************//
void com_send_string( uint8_t *data, uint8_t data_length )
{
	do{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET){;}
		USART1->DR = *data++;
		data_length --;
	}while(data_length > 0);  
}


void com_send_hex( uint8_t nmbr ){

	com_send_char('0');
	com_send_char('x');
	com_send_char(hex_lookup[nmbr >> 4]);
	com_send_char(hex_lookup[nmbr & 0x0F]);
}

void com_send_char( uint8_t data )
{    
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET){;}
		USART1->DR = data;

}
//************************************************************//
uint8_t * com_get_received_data( void ){
    return &com_buffer[0];
}


uint8_t com_get_number_of_received_bytes( void )
{
	if(Get_com_Rx_Finished_flag() ||hal_get_TimerUsart_flag())
	{
		return com_number_of_received_bytes; 
	}
	else
	{
		return 0;
	}
}

void com_reset_receiver( void )
{
 	uint8_t dummy;   
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);

	com_data_reception_finished = false;
	hal_set_TimerUsart_flag(false);
	dummy = USART_ReceiveData(USART1);
	dummy = dummy;
	com_number_of_received_bytes=0;
	for(dummy=0;dummy<COM_RX_BUFFER_SIZE;dummy++)
		com_buffer[dummy]=0;;
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}
//****************************************************************//
 void uart_rxne_isr(void)
{
	uint8_t receivedData;

	//TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
	TIM_Cmd(TIM4, DISABLE);
	TIM_SetCounter(TIM4, 0);
	//hal_set_TimerUsart_flag(false);
	
	if(USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
		USART_ReceiveData(USART1);
	if(USART_GetITStatus(USART1, USART_IT_RXNE) == RESET) return;
	receivedData = USART_ReceiveData(USART1);	//Collect data.
	
	if (com_number_of_received_bytes < COM_RX_MAX_BYTES){
		com_buffer[com_number_of_received_bytes++] = receivedData;
		
	}else{	//End of data stream.
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
		com_buffer[COM_RX_MAX_BYTES] = receivedData;	// max_byte < buf_size
		com_number_of_received_bytes = COM_RX_MAX_BYTES;
		com_data_reception_finished = true;
	}

	TIM_Cmd(TIM4, ENABLE);
	//TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}
