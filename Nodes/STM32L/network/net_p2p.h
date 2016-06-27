#ifndef NET_P2P_H
#define NET_P2P_H

#include <stdint.h>
#include "tat.h"

//typedef enum{
//	STATUS_PWRON = 0x01,
//	STATUS_ONLINE,
//	STATUS_PREOFFLINE,
//	STATUS_OFFLINE
//}nwk_status_t;
//
//typedef struct{
//	uint16_t partner_addr;
//	uint8_t status;
//	uint8_t flag;
//}nwk_local_info_t;
//
//void nwk_function_init(void);
//
//void node_opt_do_once(void);
////void node_got_wireless_msg(void);
////void node_com_rcv_data(void);
////void node_function_operation(void);
////tat_status_t node_send_command(uint16_t dest_addr, uint8_t *buf, uint8_t len);
//tat_status_t node_send_app_command(uint16_t dest_addr, uint8_t *buf, uint8_t len, uint8_t retries);
//tat_status_t node_send_net_command(uint16_t dest_addr, uint8_t *buf, uint8_t len, uint8_t retries);

// Miao
bool radio_send_frame(uint8_t length, uint8_t *data, uint8_t retry);
bool node_rf_send(uint16_t dst, const uint8_t *buf, uint8_t len, uint8_t retry, uint8_t type);
bool node_send_app_command(uint16_t dest_addr, uint8_t *buf, uint8_t len, uint8_t retry);
bool node_send_net_command(uint16_t dest_addr, uint8_t *buf, uint8_t len, uint8_t retry);

// RF messages
//void send_shutdown_cmd(uint16_t);
//void send_intv_cmd(void);
//void send_restart_cmd(uint16_t nodeid, uint8_t subcmd);
#endif

