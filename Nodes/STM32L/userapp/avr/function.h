#ifndef FUNCTION_H
#define FUNCTION_H

#include "hal.h"

bool trx_init( void );
//void trx_end_handler( void)	;
//void trx_end_handler( uint32_t time_stamp );
void pkt_filter(uint32_t);
void rx_pool_init( void );
void trx_frame_init(void);
hal_rx_frame_t * hand_rx_pool_item(void);


void system_net_Parameter_init(void);
void systeml_Hardware_init(void);
void system_init(void);


void node_hand_wireless_msg(void);
void node_hand_revcom_msg(uint8_t rcv_data_length);
bool node_tx_with_retry(uint8_t*data,uint8_t trx_lenght,uint8_t times);
void node_rf_send_data(uint8_t *din,uint8_t rec_lengh);

uint8_t get_rx_used_flag(void);
bool get_rx_over_flag(void);
bool  get_rx_hand_flag(void);

void test_com(uint8_t len);
void node_function_test(void);



#endif
