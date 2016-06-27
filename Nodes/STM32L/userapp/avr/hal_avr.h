#ifndef HAL_AVR_H
#define HAL_AVR_H
/*============================ INCLUDE =======================================*/
#include <stdint.h>
#include <stdbool.h>

#include "stm32l1xx.h"
#include "stm32l1xx_gpio.h"
#include "include.h"
#include "common_def.h"

/*============================ MACROS ========================================*/

#define HAL_SLP_TR              ( GPIO_Pin_6 )
#define HAL_PORT_SLP_TR         ( GPIOB)       
#define hal_set_slptr_high( )      GPIO_SetBits(HAL_PORT_SLP_TR, HAL_SLP_TR)
#define hal_set_slptr_low( )       GPIO_ResetBits(HAL_PORT_SLP_TR, HAL_SLP_TR)
#define hal_get_slptr( ) 		GPIO_ReadInputDataBit(HAL_PORT_SLP_TR, HAL_SLP_TR)

#define HAL_RF_RST				( GPIO_Pin_5 )	
#define HAL_PORT_RST			( GPIOB)
#define hal_set_rst_high( )		GPIO_SetBits(HAL_PORT_RST, HAL_RF_RST)
#define hal_set_rst_low( )		GPIO_ResetBits(HAL_PORT_RST, HAL_RF_RST)
#define hal_get_rst( )			GPIO_ReadInputDataBit(HAL_PORT_RST, HAL_RF_RST)


#define HAL_PORT_SPI            ( GPIOA )
#define HAL_SS_PIN                ( GPIO_Pin_4 )
#define HAL_SCK_PIN               ( GPIO_Pin_5 )
#define HAL_MOSI_PIN             ( GPIO_Pin_7  )
#define HAL_MISO_PIN             ( GPIO_Pin_6  )

#define HAL_SS_HIGH( )			GPIO_SetBits(HAL_PORT_SPI, HAL_SS_PIN)
#define HAL_SS_LOW( )			GPIO_ResetBits(HAL_PORT_SPI, HAL_SS_PIN)


//#define HAL_PORT_NET_LED		( GPIOB )	// PB7
//#define HAL_PIN_NET_LED			( GPIO_Pin_7)
//
//#define HAL_PORT_DATA_LED		( GPIOB )	// PB2
//#define HAL_PIN_DATA_LED		( GPIO_Pin_2 )

#define hal_set_net_led()		GPIO_SetBits(HAL_PORT_NET_LED, HAL_PIN_NET_LED)
#define hal_clear_net_led()		GPIO_ResetBits(HAL_PORT_NET_LED, HAL_PIN_NET_LED)

#define hal_set_data_led()		GPIO_SetBits(HAL_PORT_DATA_LED, HAL_PIN_DATA_LED)
#define hal_clear_data_led()	GPIO_ResetBits(HAL_PORT_DATA_LED, HAL_PIN_DATA_LED)

// Miao
// toggle PB8 to power on/off flash
#define flash_on()		(GPIOB->BSRRL = GPIO_Pin_8)
#define flash_off()		(GPIOB->BSRRH = GPIO_Pin_8)


//#define HAL_ENABLE_INPUT_CAPTURE_INTERRUPT( ) ( TIMSK1 |= ( 1 << ICIE1 ) )
//#define HAL_DISABLE_INPUT_CAPTURE_INTERRUPT( ) ( TIMSK1 &= ~( 1 << ICIE1 ) )

//#define HAL_ENABLE_OVERFLOW_INTERRUPT( ) ( TIMSK1 |= ( 1 << TOIE1 ) )
//#define HAL_DISABLE_OVERFLOW_INTERRUPT( ) ( TIMSK1 &= ~( 1 << TOIE1 ) )

/*! \brief  Enable the interrupt from the radio transceiver.
 *
 *  \ingroup hal_avr_api
 */
//#define hal_enable_trx_interrupt( ) ( HAL_ENABLE_INPUT_CAPTURE_INTERRUPT( ) )

//#define hal_disable_trx_interrupt( ) ( HAL_DISABLE_INPUT_CAPTURE_INTERRUPT( ) )


/*============================ TYPDEFS =======================================*/
/*============================ PROTOTYPES ====================================*/

#endif
/*EOF*/
