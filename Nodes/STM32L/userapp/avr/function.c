/*============================ INCLDUE =======================================*/
#include "include.h"
//#include "sal.h"
#include <stdint.h>
#include <stdbool.h>

#include "flash.h"
#include "sensor_tmp102.h"
#include "stm32_timer.h"
#include "hal_avr.h"
#include "mydef.h"
#include "net_layer.h"
#include "app_layer.h"
#include "bat_monitor.h"
#include "sensor_gpio.h"


/*============================ VARIABLES =====================================*/
//uint8_t tx_frame[ 127 ]; //!< Buffer used to build TX frames. (Size must be max PSDU length.)
//hal_rx_frame_t rx_pool;//[ RX_POOL_SIZE ]; //!< Pool of hal_rx_frame_t's.
//hal_rx_frame_t *rx_pool_point; //!< Pointer to start of pool.
//hal_rx_frame_t rx_pool[ RX_POOL_SIZE ]; //!< Pool of hal_rx_frame_t's.
//hal_rx_frame_t *rx_pool_start; //!< Pointer to start of pool.
//hal_rx_frame_t *rx_pool_end; //!< Pointer to end of pool.
//hal_rx_frame_t *rx_pool_head; //!< Pointer to next hal_rx_frame_t it is possible to write.
//hal_rx_frame_t *rx_pool_tail; //!< Pointer to next hal_rx_frame_t that can be read from the pool.
//uint8_t rx_pool_items_free; //!< Number of free items (hal_rx_frame_t) in the pool.
//uint8_t rx_pool_items_used; // !< Number of used items.
//bool rx_pool_overflow_flag; //!< Flag that is used to signal a pool overflow.

//static bool rx_flag; //!< Flag used to mask between the two possible TRX_END events.
//static uint8_t  Com_bandrate;
//static uint8_t *rx_frame_tmp;
//bool rx_hand_flag;

/*uint8_t aes_key[16]  = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint8_t aes_key2[16]  = {2,5,6,4,8,6,10,7,9,12,1,14,17,18,16,15};
uint8_t aes_key1[16];
uint8_t aes_output[16];
uint8_t aes_output1[16];
uint8_t aes_tmp_out[16];*/
//uint8_t test_arry[10]={0,1,2,3,4,5,6,7,8,9};

//static uint16_t tmp102_value;
//uint8_t tmp_value[2];
//uint8_t trx_frame[20];

/*============================ PROTOTYPES ====================================*/

 bool trx_init( void )
{
	
	/*while(true)
	{
		test_spi_pro();
	}*/
	trx_config();
	if (tat_init() != TAT_SUCCESS)
	{
		return false;
	}

	//if (tat_set_operating_channel( RF212_WPAN_CHANNLE3 ) != TAT_SUCCESS) RF212_CHANNEL_CN3
	if (tat_set_operating_channel(RF212_AREA_AMERICA, RF212_CHANNEL_NA0) != TAT_SUCCESS) 
	//if (tat_set_operating_channel(RF212_AREA_CHINA,RF212_CHANNEL_CN3) != TAT_SUCCESS) 
	{
		return false;    
	}
	
	tat_set_modulation_datarate( BPSK_40 );//;OQPSK_SIN_500;;BPSK_40;OQPSK_SIN_RC_100
	tat_set_tx_power_level(WPAN_TXPWR_P10DBM, R212_PABOOST_ON);//WPAN_TXPWR_P10DBM

	if (tat_set_clock_speed( true, 0 ) != TAT_SUCCESS) 
	{
	        return false; 
	}

    /*Set up the extended modes:*/
    //RX_AACK:
    tat_set_short_address(NODE_ADDR); //Short Address.
    tat_set_pan_id( PAN_ID ); //PAN ID.
    tat_set_device_role( false ); // No Coordintor support is necessary.
    
    //TX_ARET:
    tat_configure_csma( 234, 0xE2 ); // Default CSMA_SEED_0, MIN_BE = 3, MAX_CSMA_RETRIES = , and CSMA_SEED_1 = 
    //tat_configure_csma( 4,3,4,234); 
    //Both Modes:
    tat_use_auto_tx_crc( true ); //Automatic CRC must be enabled.
	//hal_set_trx_end_event_handler( trx_end_handler ); // Event handler for TRX_END events.
    hal_set_trx_end_event_handler(pkt_filter); // Event handler for TRX_END events.
        
	hal_register_read(RG_IRQ_STATUS);    /* clear pending irqs, dummy read */

	//sal_aes_restart();

	//sal_aes_setup(aes_key,AES_MODE_ECB,AES_DIR_ENCRYPT);

	return true;

}    

//void rx_pool_init( void )
//{
//
//    rx_pool_start = rx_pool;
//    rx_pool_end = &rx_pool[ RX_POOL_SIZE - 1 ];
//    
//    rx_pool_head = rx_pool_start;
//    rx_pool_tail = rx_pool_end;
//    
//    rx_pool_items_free = RX_POOL_SIZE;
//    rx_pool_items_used = 0;
//    
//    rx_pool_overflow_flag = false;
//	/*rx_pool_point = &rx_pool;
//	rx_pool_items_used = 0;*/
//    
//}

void pkt_filter(uint32_t time_stamp)
{
	hal_rx_frame_t rx_pkt;

	// upload the received frame
	hal_frame_read(&rx_pkt);

	// then check the crc
	if (rx_pkt.crc == true) { // valid pkt
		// read rssi value
		gRSSI = tat_get_ed_value();
		#ifdef RF_TEST
		debug_print(&gRSSI, sizeof(gRSSI));
		#endif // RF_TEST
		// classify pkt type
		if (rx_pkt.data[FF_SUBTYPE_CNT] == FUSR_APP) { // app layer
			push_app_pool(rx_pkt.data + RF_APP_OFFSET, rx_pkt.length-RF_APP_OFFSET-RF_APP_TAIL_LEN);
		} else if (rx_pkt.data[FF_SUBTYPE_CNT] == FUSR_NET) { // net layer
			push_net_pool(rx_pkt.data +	RF_APP_OFFSET, rx_pkt.length-RF_APP_OFFSET-RF_APP_TAIL_LEN);
		} else {
			//debug_info_output("UNKNOWN!!!\r\n");
		}
	} else {
		//debug_info_output("crc error!!!\r\n");
	}
}

//void trx_end_handler( uint32_t time_stamp )
////void trx_end_handler( void)	
//{
//    
//    if (rx_flag == true) {
//        
//        //Check if these is space left in the rx_pool.
//        if (rx_pool_items_free == 0) {
//            rx_pool_overflow_flag = true;
//        } else {
//        
//            //Space left, so upload the received frame.
//            hal_frame_read( rx_pool_head );
//            
//            //Then check the CRC. Will not store frames with invalid CRC.
//            if (rx_pool_head->crc == true) {
//                
//                //Handle wrapping of rx_pool.
//                if (rx_pool_head == rx_pool_end) {
//                    rx_pool_head = rx_pool_start;
//                } else {
//                    ++rx_pool_head;
//                } // end: if (rx_pool_head == rx_pool_end) ...
//                
//                --rx_pool_items_free;
//                ++rx_pool_items_used;
//            } // end: if (rx_pool_head->crc == true) ...
//        } // end: if (rx_pool_items_free == 0) ...
//    } // end:  if (rx_flag == true) ...
//     
//	/*if (rx_flag == true) 
//	{
//		hal_frame_read( rx_pool_point );
//		if ((rx_pool_point->crc) == true) 
//		{
//			rx_pool_items_used=1;
//		} 
//	}*/
//	//rx_pool_point=hand_rx_pool_item();
//}

//void trx_frame_init(void)
//{
////	uint8_t i;
//	/*Pre Build Header of IEEE 802.15.4 Data frame.*/
//	tx_frame[ 0 ] = 0x61; //FCF.
//	tx_frame[ 1 ] = 0x88; //FCF.
//	                       //Sequence number set during frame transmission.
//	tx_frame[ 3 ] = PAN_ID & 0xFF; //Dest. PANID.
//	tx_frame[ 4 ] = (PAN_ID >> 8 ) & 0xFF; //Dest. PANID.
//	tx_frame[ 5 ] = DEST_ADDRESS & 0xFF; //Dest. Addr.
//	tx_frame[ 6 ] = (DEST_ADDRESS >> 8 ) & 0xFF; //Dest. Addr.
//	tx_frame[ 7 ] = SHORT_ADDRESS & 0xFF; //Source Addr.
//	tx_frame[ 8 ] = (SHORT_ADDRESS >> 8 ) & 0xFF; //Source Addr.
//
//}



//hal_rx_frame_t * hand_rx_pool_item(void)
//{
//	//Handle wrapping of rx_pool.
//	if (rx_pool_tail == rx_pool_end) {
//	    rx_pool_tail = rx_pool_start;
//	} else {
//	    ++rx_pool_tail;
//		
//	} // end: if (rx_pool_tail == rx_pool_end) ...
//
//	//Turn interrupts off for a short while to protect when status
//	//information about the rx_pool is updated.
//	cli( );
//
//	++rx_pool_items_free;
//	--rx_pool_items_used;
//	//rx_hand_flag=true;
//	sei( );
//	//rx_pool_point = &rx_pool;
//	//rx_pool_items_used = 0;
//	return rx_pool_tail;
//}
//*******************************************************************//


void system_net_Parameter_init(void)
{
	trx_frame_init();
	//rx_pool_init( );

	// init network pools (Miao)
	init_net_pool();
	init_app_pool();
}


void systeml_Hardware_init(void)
{
	/* Configure the Priority Group to 2 bits */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	//#if (NODE_ADDRESS == NODE_WRKSTN) || defined(MY_DEBUG)
	// com port
	if (N_WRKSTN == gConfig.gNodeType) { // wrkstn
		com_init(BUD_115200); // com port
	} else { // tmpsnr
		#ifdef MY_DEBUG
		com_init(BUD_115200); // com port
		#endif
	}
	//#endif
	hal_init(0);
	trx_init();	 // RF init
	flash_init(); // flash (Miao)
	tmp102_init_config(); // TMP102
	hal_rtc_init(); // init RTC (set date/time...)

	// electrode
	//electrode_config();
	bat_mon_config();

	//rx_flag = true;
	//PWR_WakeUpPinCmd(PWR_WakeUpPin_1,ENABLE); // Miao: enable wkup
	while((tat_set_trx_state(RX_AACK_ON)!=TAT_SUCCESS)){
		delay_ms(1);
	}

	// init hall switch
	//ss_hall_switch_init();
}

void system_init(void)
{
	hal_system_RCC_init();// systrm RCC
//	hal_mco_output_enable();// RF_clock
	system_net_Parameter_init(); // init network configurations
	hal_flash_power_config();
	systeml_Hardware_init();//Hardware and RF init

	
//	//rx_flag = true;
//	//PWR_WakeUpPinCmd(PWR_WakeUpPin_1,ENABLE); // Miao: enable wkup
//	while((tat_set_trx_state(RX_AACK_ON)!=TAT_SUCCESS)){
//		delay_ms(1);
//	}
	
	sei();
}

//*****************************************************************//
//void node_hand_wireless_msg(void)
//{
//	//sal_aes_restart();
//	//sal_aes_setup(aes_key,AES_MODE_ECB,AES_DIR_DECRYPT);
//	
//	rx_pool_point=hand_rx_pool_item();
//	com_send_string( &(rx_pool_point->data[ 9 ]), ((rx_pool_point->length) - 9- 2 ) );//rx_pool_point
//	//rx_pool_init();
//	//rx_hand_flag=false;
//}
//*******************************************************************//
//bool node_tx_with_retry(uint8_t*data,uint8_t trx_lenght,uint8_t times)
//{
//	uint8_t retried_times=times;
//	//uint8_t test;
//	bool opt_result=true;
//	while(tat_set_trx_state( TX_ARET_ON ) != TAT_SUCCESS)
//	{
//		delay_us(100);
//	}
//	
//	if(tat_send_data_with_retry( trx_lenght, data, retried_times )!=TAT_SUCCESS)
//	{	
//		opt_result=false;	
//	}
//	
//	while(tat_set_trx_state( RX_AACK_ON )!=TAT_SUCCESS) 
//	{
//		delay_us(100);
//	}
//	return opt_result;
//}

////****************************************************************//
//void node_rf_send_data(uint8_t *din,uint8_t rec_length)
//{
//	uint8_t frame_sequence_number=0 ;
//	uint8_t tx_frame_length ;
//	uint8_t i;
//	uint8_t d[4] = {0, 0, 0, 0};
//
//	i=0;
//	tx_frame_length = 9; // Length of prebuilt frame header.
//	tx_frame[ 2 ] = frame_sequence_number++; //Sequence Number.
//	//Copy data into the TX frame buffer.
//	for(;rec_length>0;rec_length--)
//	{
//		tx_frame[ tx_frame_length++ ] = din[i];
//		d[2+i%2] = din[i];
//		i++;
//	}
//	//rx_flag = false;
//
//	if(node_tx_with_retry(tx_frame,tx_frame_length+2,1)!=true)
//	{
//		//com_send_char(hal_register_read( RG_PHY_ED_LEVEL ));
//		//com_send_char('A');
//		//com_send_hex(hal_subregister_read( SR_TRAC_STATUS ));
//		d[0] = hal_subregister_read(SR_TRAC_STATUS);
//	}
//
//	com_send_string(d, sizeof(d));	
//	
//}

////***************************************************************//
//void node_hand_revcom_msg(uint8_t rcv_data_length)
//{
//	//uint8_t frame_sequence_number ;
//	//uint8_t tx_frame_length ;
////	uint8_t i;
////	TCCR0B = 0x00;
//	rx_frame_tmp = com_get_received_data( );
//	//rx_flag = false;
//	
//	tmp102_read_temperature(&tmp102_value);
//	node_rf_send_data((uint8_t*)&tmp102_value, sizeof(tmp102_value));
//	//node_rf_send_data(rx_frame_tmp,rcv_data_length);
//	//com_send_string(rx_frame_tmp, rcv_data_length);
//
//	//rx_flag = true; // Set the flag back again. Only used to protec the frame transmission.
//	com_reset_receiver( );
//	//hal_clear_timer0_flag();
//}
////********************************************************************//

//void test_com(uint8_t len)
//{
//	rx_frame_tmp = com_get_received_data( );
//	com_send_string(rx_frame_tmp, len);
//	//com_send_string(test_arry,10);
//	com_reset_receiver( );
//	//delay_ms(500);
//}

//void node_function_test(void)
//{
//
//	uint8_t i;
//
//	//test_spi_pro();
//	
//	if(ss_get_hall_switch_status())
//	{
//		if(node_test_flash())
//		{
//			for(i=0;i<3;i++)
//			{
//				delay_ms(500);
//				delay_ms(500);
//			}
//		}
//		ss_set_hall_switch_status(0);
//		//test
//		tmp102_read_temp_value(ADDR_GND, &tmp102_value);
//		tmp_value[0]=(tmp102_value>>8)&0xff;
//		tmp_value[1]=(tmp102_value)&0xff;
//		com_send_string((uint8_t*)tmp_value,2);
//		/*for(i=0;i<3;i++)
//		{
//			delay_ms(500);
//			delay_ms(500);
//		}*/
//		
//		
//	}
//}

//*****************************************************************//
//uint8_t get_rx_used_flag(void)
//{
//	return rx_pool_items_used;
//}
//
//bool get_rx_over_flag(void)
//{
//	return rx_pool_overflow_flag;
//}

/*bool  get_rx_hand_flag(void)
{
	return rx_hand_flag;
}*/
