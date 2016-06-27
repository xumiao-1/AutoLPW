#ifndef TAT_H
#define TAT_H
/*============================ INCLUDE =======================================*/
#include <stdint.h>
#include <stdbool.h>
#if(RF_PLATFORM==ZGB_LINK_230)
#include "at86rf230.h"
#elif(RF_PLATFORM==ZGB_LINK_212)
#include "at86rf212.h"
#elif(RF_PLATFORM==ZGB_LINK_231)
#include "at86rf231.h"
#endif
#include "compiler.h"
#include "return_val.h"
//#include "tal_internal.h"
/*============================ MACROS ========================================*/
#define MIN_ED_THRESHOLD                  ( 0 )//R212 Modify
#define MAX_ED_THRESHOLD                  ( 15 )//R212 Modify
#define MAX_TX_FRAME_LENGTH               ( 127 ) //!< 127 Byte PSDU. //R212 Modify

#if(RF_PLATFORM ==ZGB_LINK_212)
#define SUPPORTED_PART_NUMBER                   ( 2 )
#define SUPPORTED_MANUFACTURER_ID               ( 31 )
#define RF212_SUPPORTED_INTERRUPT_MASK          ( 0xFF )

#define BATTERY_MONITOR_HIGHEST_VOLTAGE         ( 15 )
#define BATTERY_MONITOR_VOLTAGE_UNDER_THRESHOLD ( 0 )
#define BATTERY_MONITOR_HIGH_VOLTAGE            ( 1 )
#define BATTERY_MONITOR_LOW_VOLTAGE             ( 0 )

#define FTN_CALIBRATION_DONE                    ( 0 )
#define PLL_DCU_CALIBRATION_DONE                ( 0 )
#define PLL_CF_CALIBRATION_DONE                 ( 0 )
#define mac_i_pan_coordinator               (0x0B)

#define RF212_MIN_CHANNEL                       ( 110 )//R212 Modify
#define RF212_MAX_CHANNEL                       ( 170 )//R212 Modify
//#define RF212_WPAN_MODEL                        (1) //CC_BAND = 1  //R212 Modify
#define R212_PABOOST_ON   ( true)          //R212 Modify
#define R212_PABOOST_OFF  (false)          //R212 Modify
/*
 * Default value of backoff exponent used while performing csma ca
 */
#define TAL_MINBE_DEFAULT                   (0x03)   //R212 Modify
/*
 * Default value of maximum csma ca backoffs
 */
#define TAL_MAX_CSMA_BACKOFFS_DEFAULT       (0x04)

//typedef enum//R212 Modify
//{
//	 RF212_WPAN_CHANNLE0 =  (110),  //CC_NUMBER_VALUE
//	 RF212_WPAN_CHANNLE1 =  (130), //CC_NUMBER_VALUE
//	 RF212_WPAN_CHANNLE2 =  (150),  //CC_NUMBER_VALUE
//	 RF212_WPAN_CHANNLE3 =  (170) //CC_NUMBER_VALUE
//}wpan_channel_t;

typedef enum//R212 Modify
{
	 WPAN_TXPWR_P10DBM = (0xE1),
	 WPAN_TXPWR_P5DBM =  (0x84),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_P4DBM =  (0x85),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_P3DBM =  (0x42),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_P2DBM =  (0x22),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_P1DBM =  (0x23),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_P0DBM =  (0x24),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_N1DBM =  (0x25),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_N2DBM =  (0x04),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_N3DBM =  (0x05),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_N4DBM =  (0x06),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_N5DBM =  (0x07),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_N6DBM =  (0x08),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_N7DBM =  (0x09),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_N8DBM =  (0x0A),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_N9DBM =  (0x0B),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_N10DBM =  (0x0C),  //PHY-TX-POWER VALUE
	 WPAN_TXPWR_N11DBM =  (0x0D)  //PHY-TX-POWER VALUE
}wpan_oqpsk_txpower_t;

typedef enum //R212 Modify
{
	BPSK_20                                   = 0x00,//IEEE 802.15.4
	BPSK_40                                   = 0x04,//IEEE 802,15.4
	OQPSK_SIN_RC_100                 = 0x08,//IEEE 802.15.4-2006
	OQPSK_SIN_RC_200                 = 0x09,//特有
	OQPSK_SIN_RC_400_SCR_ON   = 0x2A,//特有，启用扰码器
	OQPSK_SIN_RC_400_SCR_OFF = 0x0A,//特有，关闭扰码器	
	OQPSK_SIN_250                       = 0x0C,//IEEE 802.15.4-2006
	OQPSK_SIN_500                       = 0x0D,//特有
	OQPSK_SIN_1000_SCR_ON       = 0x2E,//特有，启用扰码器
	OQPSK_SIN_1000_SCR_OFF     = 0x0E,//特有，关闭扰码器	
	OQPSK_RC_250                         = 0x1C,//IEEE 802.15.4c China
	OQPSK_RC_500                         = 0x09,//特有
	OQPSK_RC_1000_SCR_ON         = 0x3E,//特有，启用扰码器
	OQPSK_RC_1000_SCR_OFF       = 0x1E//特有，关闭扰码器	
}rf212_modulation_datarate_t;

#else
/*============================ MACROS ========================================*/
#define SUPPORTED_PART_NUMBER                   ( 2 )
#define RF230_REVA                              ( 1 )
#define RF230_REVB                              ( 2 )
#define SUPPORTED_MANUFACTURER_ID               ( 31 )
#define RF230_SUPPORTED_INTERRUPT_MASK          ( 0xCF )
#define RF231_SUPPORTED_INTERRUPT_MASK          ( 0xFF )

#define RF23x_MIN_CHANNEL                       ( 11 )
#define RF23x_MAX_CHANNEL                       ( 26 )

#define TX_PWR_3DBM                             ( 0 )
#define TX_PWR_17_2DBM                          ( 15 )

#define BATTERY_MONITOR_HIGHEST_VOLTAGE         ( 15 )
#define BATTERY_MONITOR_VOLTAGE_UNDER_THRESHOLD ( 0 )
#define BATTERY_MONITOR_HIGH_VOLTAGE            ( 1 )
#define BATTERY_MONITOR_LOW_VOLTAGE             ( 0 )

#define FTN_CALIBRATION_DONE                    ( 0 )
#define PLL_DCU_CALIBRATION_DONE                ( 0 )
#define PLL_CF_CALIBRATION_DONE                 ( 0 )

#endif
/*============================ TYPEDEFS ======================================*/

/*! \brief  This macro defines the start value for the TAT_* status constants.
 *          
 *          It was chosen to have this macro so that the user can define where
 *          the status returned from the TAT starts. This can be useful in a
 *          system where numerous drivers are used, and some range of status codes
 *          are occupied.
 * 
 *  \see tat_status_t
 *  \ingroup tat
 */
#define TAT_STATUS_START_VALUE                  ( 0x40 )

//=======add by MIAO====
typedef enum{
	RF212_AREA_CHINA = 1,
	RF212_AREA_EUROPE,
	RF212_AREA_AMERICA
}wpan_area_t;

typedef enum//R212 Modify
{
	RF212_CHANNEL_EU  = (113),	// 欧洲频段(868.3MHz)
	RF212_CHANNEL_NA0 = (30),	// 北美频段0~10 (906~924MHz)
	RF212_CHANNEL_NA1 = (50),
	RF212_CHANNEL_NA2 = (70),
	RF212_CHANNEL_NA3 = (90),
	RF212_CHANNEL_NA4 = (110),
	RF212_CHANNEL_NA5 = (130),
	RF212_CHANNEL_NA6 = (150),
	RF212_CHANNEL_NA7 = (170),
	RF212_CHANNEL_NA8 = (190),
	RF212_CHANNEL_NA9 = (210),
	RF212_CHANNEL_CN0 = (110),  	// 中国频段0~3 (780~786MHz)
	RF212_CHANNEL_CN1 = (130), 
	RF212_CHANNEL_CN2 = (150), 
	RF212_CHANNEL_CN3 = (170), 
	RF23x_CHANNEL_11 = (11),	// 2405MHz
 	RF23x_CHANNEL_12 = (12),	// 2410MHz
 	RF23x_CHANNEL_13 = (13),	// ...
	RF23x_CHANNEL_14 = (14),
	RF23x_CHANNEL_15 = (15),
	RF23x_CHANNEL_16 = (16),
	RF23x_CHANNEL_17 = (17),
	RF23x_CHANNEL_18 = (18),
	RF23x_CHANNEL_19 = (19),
	RF23x_CHANNEL_20 = (20),
	RF23x_CHANNEL_21 = (21),
	RF23x_CHANNEL_22 = (22),
	RF23x_CHANNEL_23 = (23),
	RF23x_CHANNEL_24 = (24),
	RF23x_CHANNEL_25 = (25),
	RF23x_CHANNEL_26 = (26)	// 2480MHz
}wpan_channel_t;
//=======end============

/**
 * CCA Modes of the transceiver
 */
 typedef enum trx_cca_mode_tag
{
    CCA_MODE0 = 0, /* used for RF231 */
    CCA_MODE1 = 1,
    CCA_MODE2 = 2,
    CCA_MODE3 = 3
}
trx_cca_mode_t;


/*! \brief  This enumeration defines the possible return values for the TAT API 
 *          functions.
 *          
 *          These values are defined so that they should not collide with the
 *          return/status codes defined in the IEEE 802.15.4 standard.
 *
 *  \ingroup tat
 */
typedef enum{
    //!< The requested service was performed successfully.
    TAT_SUCCESS = TAT_STATUS_START_VALUE,
    //!< The connected device is not an Atmel AT86RF230.
    TAT_UNSUPPORTED_DEVICE,
    //!< One or more of the supplied function arguments are invalid.
    TAT_INVALID_ARGUMENT,
    //!< The requested service timed out.
    TAT_TIMED_OUT,
    //!< The end-user tried to do an invalid state transition.
    TAT_WRONG_STATE,
    //!< The radio transceiver is busy receiving or transmitting.
    TAT_BUSY_STATE,
    //!< The requested state transition could not be completed.
    TAT_STATE_TRANSITION_FAILED,
    //!< Channel in idle. Ready to transmit a new frame.
    TAT_CCA_IDLE,
    //!< Channel busy.
    TAT_CCA_BUSY,
    //!< Transceiver is busy receiving or transmitting data.
    TAT_TRX_BUSY,
    //!< Measured battery voltage is lower than voltage threshold.
    TAT_BAT_LOW,
    //!< Measured battery voltage is above the voltage threshold.
    TAT_BAT_OK,
    //!< The CRC failed for the actual frame.
    TAT_CRC_FAILED,
    //!< The channel access failed during the auto mode.
    TAT_CHANNEL_ACCESS_FAILURE,
    //!< No acknowledge frame was received.
    TAT_NO_ACK,
}tat_status_t;

/*! \brief  This enumeration defines the possible modes available for the 
 *          Clear Channel Assessment algorithm.
 *
 *          These constants are extracted from the datasheet.
 *
 *  \ingroup tat
 */
typedef enum{
    //!< Use energy detection above threshold mode.
    CCA_ED                    = 0,
    //!< Use carrier sense mode.
    CCA_CARRIER_SENSE         = 1,
    //!< Use a combination of both energy detection and carrier sense.
    CCA_CARRIER_SENSE_WITH_ED = 2
}tat_cca_mode_t;

/*! \brief  This enumeration defines the possible CLKM speeds.
 *
 *          These constants are extracted from the datasheet.
 *
 *  \ingroup tat
 */
/*#if(RF_PLATFORM==ZGB_LINK_230)
typedef enum{

    CLKM_DISABLED      = 0,
    CLKM_1MHZ          = 1,
    CLKM_2MHZ          = 2,
    CLKM_4MHZ          = 3,
    CLKM_8MHZ          = 4,
    CLKM_16MHZ         = 5    
}tat_clkm_speed_t;
#endif
*/
/*============================ PROTOTYPES ====================================*/
void test_spi_pro(void);

tat_status_t tat_init( void );
uint8_t tat_get_operating_channel( void );
void trx_config(void);
#if(RF_PLATFORM ==ZGB_LINK_212)
//tat_status_t tat_set_operating_channel( wpan_channel_t channel );//R212 Modify
tat_status_t tat_set_operating_channel(wpan_area_t area, wpan_channel_t channel );//R212 Modify
uint8_t tat_get_tx_power_level( void );//R212 Modify
tat_status_t tat_set_tx_power_level( wpan_oqpsk_txpower_t power_level ,bool PA_BOOST);//R212 Modify
//void trx_config(void);//R212 Modify
retval_t tat_set_modulation_datarate(rf212_modulation_datarate_t mode);//R212 Modify
#else
tat_status_t tat_set_operating_channel( uint8_t channel );
tat_status_t tat_set_tx_power_level( uint8_t power_level );
#endif

tat_status_t tat_do_ed_scan( uint8_t *ed_level );
uint8_t tat_get_cca_mode( void );
uint8_t tat_get_ed_threshold( void );
uint8_t tat_get_ed_value(void);
tat_status_t tat_set_cca_mode( uint8_t mode, uint8_t ed_threshold );
tat_status_t tat_do_cca( void );
tat_status_t  tat_get_rssi_value( uint8_t *rssi );

uint8_t tat_batmon_get_voltage_threshold( void );
uint8_t tat_batmon_get_voltage_range( void );
tat_status_t tat_batmon_configure( bool range, uint8_t voltage_threshold );
tat_status_t tat_batmon_get_status( void );

uint8_t tat_get_clock_speed( void );//R212 Modify
tat_status_t tat_set_clock_speed( bool direct, uint8_t clock_speed );//R212 Modify

tat_status_t tat_calibrate_filter( void );
tat_status_t tat_calibrate_pll( void );

uint8_t tat_get_trx_state( void );
tat_status_t tat_set_trx_state( uint8_t new_state );//R212 Modify
//tal_trx_status_t set_trx_state(trx_cmd_t trx_cmd);
tat_status_t tat_enter_sleep_mode( void );
tat_status_t tat_leave_sleep_mode( void );
void tat_reset_state_machine( void );
retval_t trx_reset(void);//R212 Modify

void tat_use_auto_tx_crc( bool auto_crc_on );
 tat_status_t tat_send_data( uint8_t data_length, uint8_t *data );

uint8_t tat_get_device_role( void );
void tat_set_device_role( bool i_am_coordinator );
uint16_t tat_get_pan_id( void );
void tat_set_pan_id( uint16_t new_pan_id );
uint16_t tat_get_short_address( void );
void tat_set_short_address( uint16_t new_short_address );
void tat_get_extended_address( uint8_t *extended_address );
void tat_set_extended_address( uint8_t *extended_address );
tat_status_t tat_configure_csma( uint8_t seed0, uint8_t be_csma_seed1 );
//tat_status_t tat_configure_csma( uint8_t min_be, uint8_t frame_retry, uint8_t csma_retry,uint16_t csma_seed);
tat_status_t tat_send_data_with_retry( uint8_t frame_length, uint8_t *frame, 
                                       uint8_t retries );

void switch_pll_on(void);
bool is_sleeping( void );
#endif
/*EOF*/
