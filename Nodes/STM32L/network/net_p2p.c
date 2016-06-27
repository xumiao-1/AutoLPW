#include "net_p2p.h"
#include "at86rf212.h"
#include "config_uart_extended.h"
#include "mydef.h"


uint8_t tx_frame[ 127 ]; //!< Buffer used to build TX frames. (Size must be max PSDU length.)

void trx_frame_init(void)
{
	/*Pre Build Header of IEEE 802.15.4 Data frame.*/
	tx_frame[ 0 ] = 0x61; //FCF.
	tx_frame[ 1 ] = 0x88; //FCF.
	                       //Sequence number set during frame transmission.
	tx_frame[ 3 ] = PAN_ID & 0xFF; //Dest. PANID.
	tx_frame[ 4 ] = (PAN_ID >> 8 ) & 0xFF; //Dest. PANID.

	tx_frame[ 7 ] = NODE_ADDR & 0xFF; //Source Addr.
	tx_frame[ 8 ] = (NODE_ADDR>>8 ) & 0xFF; //Source Addr.
	tx_frame[ 9 ] = 0xFF;
	tx_frame[10] = 0x01;
}

// return 1 if not success
bool node_rf_send(uint16_t dest_addr, const uint8_t *buf, uint8_t length, uint8_t retry, uint8_t type) {
	uint8_t i=0;
	uint8_t tx_frame_length = FRAME_HEADER_LEN; // Length of prebuilt frame header.

//    packet->MAC.dst_pan = radio_get_pan();
//    packet->MAC.dst_addr = router_state->next_hop;
//    packet->MAC.src_addr = radio_get_addr();
//    packet->MAC.dsn = router_state->next_dsn++;
//    packet->MAC.fcf = FCF_ACK_REQ;
	tx_frame[ 5 ] = dest_addr & 0xFF; //Dest. Addr.
	tx_frame[ 6 ] = (dest_addr >> 8 ) & 0xFF; //Dest. Addr.
	tx_frame[FRAME_HEADER_LEN-1] = type; // frame type: app or net
	
//	tx_frame[ 2 ] = frame_sequence_number++; //Sequence Number.

	//Copy data into the TX frame buffer.
	for(; length>0; length--)
	{
		tx_frame[ tx_frame_length++ ] = buf[i];
		i++;
	}

    return radio_send_frame(tx_frame_length+2, tx_frame, retry);
}

// return true if success.
// if not, then return false
bool radio_send_frame(uint8_t length, uint8_t *data, uint8_t retry) {
    uint8_t ret = true;
    if(tat_set_trx_state(TX_ARET_ON) == TAT_SUCCESS) {

        if(tat_send_data_with_retry(length, data, retry) != TAT_SUCCESS) {
            ret = false;
        }

        tat_set_trx_state(RX_AACK_ON);
    } else {
        ret = false;
    }
    return ret;
}

bool node_send_app_command(uint16_t dest_addr, uint8_t *buf, uint8_t len, uint8_t retry)
{
	return node_rf_send(dest_addr, buf, len, retry, FUSR_APP);
}

bool node_send_net_command(uint16_t dest_addr, uint8_t *buf, uint8_t len, uint8_t retry)
{
	return node_rf_send(dest_addr, buf, len, retry, FUSR_NET);
}

// RF messages...
// power off cmd
// nodeid: the node to be turned off,
// if it's 0x0, then all nodes will be turned off
//void send_shutdown_cmd(uint16_t nodeid) {
//	static uint16_t shut_seqno = 1; // it's used in case broadcasting storm.
//								// start from 1 to ensure larger than 
//								// the one in "tmpsnr.c"
//	shut_data_t data;
//	pkt_app_t rep = {0, APP_NODE_OFF, 0xFF};
//	rep.hdr.nodeid = gConfig.gNodeAddr;
//	data.seqno = shut_seqno++;
//	data.nodeid = nodeid; // the node to be turned off
//
//	memcpy(rep.data, (uint8_t*)&data, sizeof(shut_data_t));
//	node_send_app_command(BROADCAST_ADDRESS, (uint8_t*)&rep, SIZEOF(shut_data_t), 0);
//}

//void send_restart_cmd(uint16_t nodeid, uint8_t subcmd) {
//	static uint8_t restart_seqno = 1; // it's used in case broadcasting storm.
//								// start from 1 to ensure larger than 
//								// the one in "tmpsnr.c"
//	shut_data_t data;
//	pkt_app_t pkt = {0, APP_NODE_RESTART, 0xFF};
//
//	// pkt header part
//	pkt.hdr.nodeid = gConfig.gNodeAddr;
//
//	// pkt data part
//	data.seqno = restart_seqno++;
//	data.nodeid = nodeid; // the node to be turned off
//	data.subcmd = subcmd; // subcmd: SUB_RESTART_ON/OFF/REPROG
//	memcpy(pkt.data, (uint8_t*)&data, sizeof(shut_data_t));
//
//	node_send_app_command(BROADCAST_ADDRESS, (uint8_t*)&pkt, SIZEOF(shut_data_t), 2);
//}

//void send_intv_cmd(void) {
//	static uint16_t intv_seqno = 1; // it's used in case broadcasting storm.
//								// start from 1 to ensure larger than 
//								// the one in "tmpsnr.c"
//	intv_data_t data;
//	pkt_app_t pkt = {0, APP_INTV_CMD, 0xFF};
//	pkt.hdr.nodeid = gConfig.gNodeAddr;
//	data.seqno = intv_seqno++;
//	data.intv = gConfig.APP_WKUP_INTV;
//
//	memcpy(pkt.data, (uint8_t*)&data, sizeof(intv_data_t));
//	node_send_app_command(BROADCAST_ADDRESS, (uint8_t*)&pkt, SIZEOF(intv_data_t), 0);
//}
// eof...
