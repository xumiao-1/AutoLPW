#ifndef HAL_H
#define HAL_H
/*============================ INCLUDE =======================================*/
#include <stdint.h>
#include <stdbool.h>
//#include "return_val.h"
#include "compiler.h"

/*============================ MACROS ========================================*/
#define HAL_BAT_LOW_MASK       ( 0x80 ) //!< Mask for the BAT_LOW interrupt.
#define HAL_TRX_UR_MASK        ( 0x40 ) //!< Mask for the TRX_UR interrupt.
#define HAL_TRX_END_MASK       ( 0x08 ) //!< Mask for the TRX_END interrupt.
#define HAL_RX_START_MASK      ( 0x04 ) //!< Mask for the RX_START interrupt.
#define HAL_PLL_UNLOCK_MASK    ( 0x02 ) //!< Mask for the PLL_UNLOCK interrupt.
#define HAL_PLL_LOCK_MASK      ( 0x01 ) //!< Mask for the PLL_LOCK interrupt.

#define HAL_MIN_FRAME_LENGTH   ( 0x03 ) //!< A frame should be at least 3 bytes.
#define HAL_MAX_FRAME_LENGTH   ( 0x7F ) //!< A frame should no more than 127 bytes.
/*============================ TYPDEFS =======================================*/
/*! \brief  This struct defines the rx data container.
 *
 *  \see hal_frame_read
 *
 *  \ingroup hal
 */
typedef struct{
    uint8_t length;
    uint8_t data[ HAL_MAX_FRAME_LENGTH ];
    uint8_t lqi;
    bool crc;
} hal_rx_frame_t;

//! RX_START event handler callback type. Is called with timestamp in IEEE 802.15.4 symbols and frame length.
typedef void (*hal_rx_start_isr_event_handler_t)(uint32_t const isr_timestamp, uint8_t const frame_length);

//! RRX_END event handler callback type. Is called with timestamp in IEEE 802.15.4 symbols and frame length.
typedef void (*hal_trx_end_isr_event_handler_t)(uint32_t const isr_timestamp);
/*============================ PROTOTYPES ====================================*/

void hal_en_net_led(void);
void hal_en_data_led(void);
void hal_flash_power_config(void);


//void hal_mco_output_enable(void);
void hal_system_RCC_init(void);

void hal_SPI_init(void);
void hal_Input_Capture_init(void);

void  hal_init( uint8_t time0_vlaue);
void hal_input_capture_isr(void);


void hal_reset_flags( void );
uint8_t hal_get_bat_low_flag( void );
void hal_clear_bat_low_flag( void );

uint8_t hal_get_trx_ur_flag( void );
void hal_clear_trx_ur_flag( void );

uint8_t hal_get_trx_end_flag( void );
void hal_clear_trx_end_flag( void );
hal_trx_end_isr_event_handler_t hal_get_trx_end_event_handler( void );
void hal_set_trx_end_event_handler( hal_trx_end_isr_event_handler_t trx_end_callback_handle );
void hal_clear_trx_end_event_handler( void );

uint8_t hal_get_rx_start_flag( void );
void hal_clear_rx_start_flag( void );
hal_rx_start_isr_event_handler_t hal_get_rx_start_event_handler( void );
void hal_set_rx_start_event_handler( hal_rx_start_isr_event_handler_t rx_start_callback_handle );
void hal_clear_rx_start_event_handler( void );

uint8_t hal_get_unknown_isr_flag( void );   
void hal_clear_unknown_isr_flag( void );

uint8_t hal_get_pll_unlock_flag( void );
void hal_clear_pll_unlock_flag( void );

uint8_t hal_get_pll_lock_flag( void );
void hal_clear_pll_lock_flag( void );

uint8_t hal_RF_SPI_Send_Data(uint8_t send_data);
uint8_t hal_register_read( uint8_t address );
void hal_register_write( uint8_t address, uint8_t value );
uint8_t hal_subregister_read( uint8_t address, uint8_t mask, uint8_t position );
void hal_subregister_write( uint8_t address, uint8_t mask, uint8_t position, 
                            uint8_t value );

void hal_frame_read( hal_rx_frame_t *rx_frame );
void hal_frame_write( uint8_t *write_buffer, uint8_t length );
void hal_sram_read( uint8_t address, uint8_t length, uint8_t *data );
void hal_sram_write( uint8_t address, uint8_t length, uint8_t *data );
void hal_trx_aes_wrrd(uint8_t addr, uint8_t *idata, uint8_t length);
//uint32_t hal_get_system_time( void );

bool hal_get_timer0_flag(void);
void  hal_clear_timer0_flag(void );
void hal_set_timer0_flag( void );
void Timer0_init(uint8_t TCN_vlaue);
#endif
/*EOF*/
