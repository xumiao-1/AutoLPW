#include "net_layer.h"
//#include "sys_cfg.h"
#include "net_p2p.h"
#include "function.h"
//#include "rfbuffer.h"
//#include "startup.h"

// buffer for application layer
#define NET_POOL_SIZE (POOL_SIZE)
#define MAX_HOPS (100)
#define MIN_HOPS (0)
#define PRNT_TIMEOUT (86400 * 2)	// timeout every 2d
#define CHLD_TIMEOUT (86400)	// timeout every 1d


static pkt_app_t net_pool_in[NET_POOL_SIZE];
static pkt_app_t *net_pool_start;// = net_pool_in;
static pkt_app_t *net_pool_end;// = net_pool_in + NET_POOL_SIZE - 1;
static pkt_app_t *net_pool_head;// = net_pool_in;
static pkt_app_t *net_pool_tail;// = net_pool_in + NET_POOL_SIZE - 1;
static uint8_t net_pool_items_free;// = NET_POOL_SIZE;
static uint8_t net_pool_items_used;// = 0;
static bool net_pool_overflow_flag;

// neigbhor related
#define MAX_NEIGHBORS	(8) // at most 8 neighbors
#define MAX_BEACONS	(5)
#define INT_BEACONS (1000)
static NeighborInfo tbl_neighbors[MAX_NEIGHBORS];
static uint8_t nNeighbors = 0;
static hopinfo_t myHopInfo; // my own hop info
//static hopinfo_t *pParent; // parent/next hop
static NeighborInfo* pParent; // parent/next hop


void init_net_pool(void) {
	reset_net_pool();
//	reset_beacon_array();
	init_neighbor_tbl();
}

void reset_net_pool(void) {
	net_pool_start = net_pool_in;
	net_pool_end = &net_pool_in[NET_POOL_SIZE - 1];
	net_pool_head = net_pool_start;
	net_pool_tail = net_pool_end;
	net_pool_items_free = NET_POOL_SIZE;
	net_pool_items_used = 0;
	net_pool_overflow_flag = false;
}

uint8_t get_net_pool_items(void) {
	return net_pool_items_used;
}

pkt_app_t *get_net_pool_handle(void) {
	if (net_pool_items_used == 0) return NULL;

	if (net_pool_tail == net_pool_end) {
		net_pool_tail = net_pool_start;
	} else {
		net_pool_tail++;
	}

	AVR_ENTER_CRITICAL_REGION();
	net_pool_items_free++;
	net_pool_items_used--;
	AVR_LEAVE_CRITICAL_REGION();

	// check for pool overflow
	if (net_pool_overflow_flag) {
		AVR_ENTER_CRITICAL_REGION();
		net_pool_overflow_flag = false;
		AVR_LEAVE_CRITICAL_REGION();
	}

	return net_pool_tail;
}

bool push_net_pool(uint8_t *data, uint8_t len) {
	// pool overflow
	if (net_pool_items_free == 0)	{
//		SoftFault_Handler(2);
		net_pool_overflow_flag = true;
		return false; 

	} else {
		// push into pool head
		memcpy((uint8_t*)net_pool_head, data, len);
//		set_rssi_val(net_pool_head); // set rssi value
	
		// move head pointer to next writable
		if (net_pool_head == net_pool_end)
			net_pool_head = net_pool_start;
		else
			net_pool_head++;
	
		//Turn interrupts off for a short while to protect when status
		//information about the rx_pool is updated.
		//AVR_ENTER_CRITICAL_REGION();
		net_pool_items_free--;
		net_pool_items_used++;
		//AVR_LEAVE_CRITICAL_REGION();
	
		return true;
	}
}


void neighbor_discover(void) {
	// broadcast neighbor discover req
	pkt_app_t rt_req = {0, NET_RUTE_REQ, 0};
	rt_req.hdr.nodeid = gConfig.gNodeAddr;

	#ifdef MY_DEBUG
	my_print(DBUG_MSG, "discover neighbor...\r\n", MAX_DBG_MSG);
	#endif

	node_send_net_command(BROADCAST_ADDRESS, (uint8_t*)&rt_req, sizeof(pkt_app_header_t), 0);
}

hopinfo_t* get_next_hop(void) {
	if (pParent == NULL) {
		neighbor_discover();
		return NULL;
	}

	return &(pParent->info);
}

void node_got_net_msg(pkt_app_t *pkt) {
//	#ifdef MY_DEBUG
//	debug_print("received a net layer packet...\r\n");
//	#endif
	switch (pkt->hdr.cmd) {
	case NET_RUTE_REQ: {
		if (pParent != NULL && pParent->info.nodeid != pkt->hdr.nodeid) { // reply to route request
			pkt_app_t rt_rep = {0, NET_RUTE_REP, 0};
			rt_rep.hdr.nodeid = gConfig.gNodeAddr;
			memcpy(rt_rep.data, (uint8_t*)&myHopInfo, sizeof(myHopInfo));
			node_send_net_command(pkt->hdr.nodeid, (uint8_t*)&rt_rep, SIZEOF(myHopInfo), 0);
		}
		break;
	}

	case NET_RUTE_REP: { // receive route reply
		deal_rt_reply((hopinfo_t*)(pkt->data));
		break;
	}

//	case NET_RUTE_BCN: { // receive beacons from neighbors
//		deal_rt_beacon(pkt);
//		break;
//	}

	default:
		break;
	}


}

void node_net_msg(void) {
	pkt_app_t *curPkt = get_net_pool_handle(); // get next pkt

	while (curPkt) {
		node_got_net_msg(curPkt);

		curPkt = get_net_pool_handle();
	}
}



/*=============================================
The following functions will operate on tbl_neighbors
==============================================*/
uint8_t get_nb_num(void) {
	return nNeighbors;
}

//NeighborInfo* get_nb_by_idx(uint8_t idx) {
//	return &tbl_neighbors[idx];
//}

NeighborInfo* get_nb_by_id(uint16_t id) {
	uint8_t i = 0;
	
	for ( ; i<MAX_NEIGHBORS; i++) {
		if (tbl_neighbors[i].type != NB_U && tbl_neighbors[i].info.nodeid == id)
			return &tbl_neighbors[i];
	}
	
	return NULL;	
}

/**
 * update neighbor's timestamp. Will add a new one if not existed
 * In:
 *	- id: node id
 *	- type: neighbor type
 */
//void update_nb(uint16_t id, nb_type type) {
//	NeighborInfo *nb = get_nb_by_id(id);
//	
//	if (nb != NULL) { // existing nb
//		nb->lastcnt = rtc_getcounter(); // update timestamp
////		RTC_GetDate(RTC_Format_BIN, &tbl_neighbors[nNeighbors].date);
////		RTC_GetTime(RTC_Format_BIN, &tbl_neighbors[nNeighbors].time);
//		nb->type = type;
//		if (type == NB_U) nNeighbors--;
//	} else { // new neighbor
//		hopinfo_t info;
//		info.nodeid = id;
//		if (type == NB_C)
//			info.hops = myHopInfo.hops+1; // child: myhop + 1
//		else if (type == NB_P)
//			info.hops = myHopInfo.hops-1; // parent: myhop - 1
//
//		nb = add_nb(&info, type); // neigbhor type: child
//	}
//}
void update_nb_by_id(uint16_t id, nb_type type) {
	NeighborInfo *nb = get_nb_by_id(id);

	if (nb != NULL)	// existing nb
		update_nb(nb, type);
	else { // new nb
		hopinfo_t nbInfo;
		nbInfo.nodeid = id;
		if (type == NB_C)
			nbInfo.hops = myHopInfo.hops+1; // child: myhop + 1
		else if (type == NB_P)
			nbInfo.hops = myHopInfo.hops-1; // parent: myhop - 1

		nb = add_nb(&nbInfo, type); // neigbhor type: child
	}
}

/**
 * init nb table
 */
void init_neighbor_tbl(void) {
	// first, init my own info: id, hops etc.
	uint8_t i = 0;
	for ( ; i<MAX_NEIGHBORS; i++) {
		tbl_neighbors[i].type = NB_U;
	}
	nNeighbors = 0;

	myHopInfo.nodeid = gConfig.gNodeAddr;
	//#if (NODE_ADDRESS == NODE_WRKSTN) // sink node
	if (N_WRKSTN == gConfig.gNodeType) {
		// for sink, assume its parent is itself
		myHopInfo.hops = MIN_HOPS;
		pParent = add_nb(&myHopInfo, NB_P);
		myHopInfo.hops = MIN_HOPS; // have to do it again
	
////		pParent = add_nb(&info, NB_P);
//		memcpy((uint8_t*)&tbl_neighbors[0].info, (uint8_t*)&myHopInfo, sizeof(hopinfo_t)); // hopinfo_t
//		tbl_neighbors[0].type = NB_P; // type
//		tbl_neighbors[0].lastcnt = rtc_getcounter(); // timestamp	
//		pParent = &tbl_neighbors[0];
//		nNeighbors = 1;
	//#else
	} else {
		// for sensor, assume no parent at first
		myHopInfo.hops = MAX_HOPS;
		pParent = NULL;
		//nNeighbors = 0;
	//#endif
	}
}

// update parent's info
void deal_rt_reply(hopinfo_t *info) {
	#ifdef MY_DEBUG
	my_print(DBUG_MSG, "dealing with route reply\r\n", MAX_DBG_MSG);
	#endif

	if (info->hops+1 < myHopInfo.hops) { // new parent
		#ifdef MY_DEBUG
		snprintf((char*)dbg_msg, MAX_DBG_MSG, "old nb: 0x%04X (%d hop)\r\n",
				pParent==NULL?NULL:pParent->info.nodeid,
				myHopInfo.hops);
		my_print(DBUG_MSG, dbg_msg, MAX_DBG_MSG);
		#endif

		add_nb(info, NB_P);
		set_log(L_REP_P);

		#ifdef MY_DEBUG
		snprintf((char*)dbg_msg, MAX_DBG_MSG, "new nb: 0x%04X (%d hop)\r\n",
				pParent==NULL?NULL:pParent->info.nodeid,
				myHopInfo.hops);
		my_print(DBUG_MSG, dbg_msg, MAX_DBG_MSG);
		#endif
	}
//	bool bUpdate = false; // need update parent info?
//	#ifdef MY_DEBUG
//	uint8_t buf[64] = {0};
//	#endif
//
//	NeighborInfo *nb = get_nb_by_id(info->nodeid);
//
//	#ifdef MY_DEBUG
//	sprintf((char*)buf, "# of nb: %d\r\n", nNeighbors);
//	debug_print(buf);
//	sprintf((char*)buf, "nb reply from 0x%04X (%d hop): ", info->nodeid, info->hops);
//	debug_print(buf);
//	#endif
//	if (nb != NULL) { // existing neighbor
//		#ifdef MY_DEBUG
//		debug_print("known nb\r\n");
//		#endif
//		if (nb->info.hops >= info->hops) { 
//			// update neighbor's info:
//			nb->info.hops = info->hops; // hops
//			nb->lastcnt = rtc_getcounter(); // timestamp
//			
//			bUpdate = true;
//		}
//	} else { // a new neigbhor
//		#ifdef MY_DEBUG
//		debug_print("new nb\r\n");
//		#endif
//		nb = add_nb(info, NB_U); // unknown type neighbor
//		
//		bUpdate = true;
//	}
//
//	// if update info is needed, then do it!
//	if (bUpdate && nb->info.hops+1 < myHopInfo.hops/* && nb->lqi > MAX_BEACONS/2*/) {
//		#ifdef MY_DEBUG
//		debug_print("updating parent...\r\n");
//		#endif
//		// change neighbor's type
//		if (pParent!=NULL) { // change the original parent to NB_U
//			pParent->type = NB_U;
//			nNeighbors--;
//			#ifdef MY_DEBUG
//			sprintf((char*)buf, "remove 0x%04X (%d hop)\r\n", pParent->info.nodeid, pParent->info.hops);
//			debug_print(buf);
//			#endif
//		}
//
//		if (nb->type == NB_U) nNeighbors++;
//		nb->type = NB_P; // change the new one to NB_P
//		pParent = nb; // change the parent to a new node
//		#ifdef MY_DEBUG
//		sprintf((char*)buf, "add 0x%04X (%d hop)\r\n", pParent->info.nodeid, pParent->info.hops);
//		debug_print(buf);
//		#endif
//		myHopInfo.hops = pParent->info.hops + 1;
//		#ifdef MY_DEBUG
//		sprintf((char*)buf, "my hops: %d, # of nb: %d\r\n", myHopInfo.hops, nNeighbors);
//		debug_print(buf);
//		#endif
//		
//		set_log(L_REP_P);
//	}
}


/**
 * check neigbhor table. remove those invalid neighbors
 */
void check_nb_tbl(void) {
	uint32_t tm = rtc_getcounter(); // current timestamp

	if (N_WRKSTN == gConfig.gNodeType) { // wrkstn
		uint8_t i = 0;
		for ( ; i<MAX_NEIGHBORS; i++) {
			if (tbl_neighbors[i].type == NB_C && (tm - tbl_neighbors[i].lastcnt)>CHLD_TIMEOUT) {// for sink, NB_P never expires
				// chage to NB_U type
				update_nb(&tbl_neighbors[i], NB_U);
			}
		}
	} else { // sensor
		uint8_t i = 0;
		for ( ; i<MAX_NEIGHBORS; i++) {
			if (tbl_neighbors[i].type == NB_P) {
				if ((tm - tbl_neighbors[i].lastcnt)>=PRNT_TIMEOUT) {
					// chage to NB_U type
					update_nb(&tbl_neighbors[i], NB_U);
				}
			} else if (tbl_neighbors[i].type == NB_C) {
				if ((tm - tbl_neighbors[i].lastcnt)>=CHLD_TIMEOUT) {
					// chage to NB_U type
					update_nb(&tbl_neighbors[i], NB_U);
				}
			}
		}
	}

//	// remove NB_U
//	j = 0;
//	for (i=0; i<n; i++)	{
//		if (tbl_neighbors[i].type != NB_U) {
//			tbl_neighbors[j] = tbl_neighbors[i];
//			j++;
//		} else { // NB_U
//			nNeighbors--;
//		}
//	}

//////	if (bPrntChanged) { // parent info changed, need a new one
//////		uint8_t min	= MAX_HOPS;
//////		uint8_t k = MAX_NEIGHBORS;
//////		for (i=0; i<n; i++)	{
//////			if (tbl_neighbors[i].type != NB_U) {
//////				if (tbl_neighbors[i].info.hops < min) {
//////					min = tbl_neighbors[i].info.hops;
//////					k = i;
//////				}
//////			}
//////		}
//////		if (MAX_NEIGHBORS == k)	{ // no parent found
//////			pParent = NULL;
//////		} else { // a new parent is found
//////			pParent = &tbl_neighbors[k];
//////		}
//////	}

//	uint8_t i, /*j, */n = nNeighbors;
//	//bool bPrnt = false; // track if still have parent
//	uint32_t tm = RTC_GetCounter(); // current timestamp
//	
//	for (i=0 ; i<n; i++) {
//		if (gConfig.gNodeType == N_WRKSTN) {
//			if (tbl_neighbors[i].type == NB_C && (tm - tbl_neighbors[i].lastcnt)>CHLD_TIMEOUT) {// for sink, NB_P never expires
//				// chage to NB_U type
//				tbl_neighbors[i].type = NB_U;
//			}
//		} else {
//			if ((tbl_neighbors[i].type == NB_P && (tm - tbl_neighbors[i].lastcnt)>PRNT_TIMEOUT) ||
//				(tbl_neighbors[i].type == NB_C && (tm - tbl_neighbors[i].lastcnt)>CHLD_TIMEOUT)) {
//				// chage to NB_U type
//				tbl_neighbors[i].type = NB_U;
//			}
//		}
//	}
//
//	// remove all those elements that are NB_U
//	i = j = 0;
//	while (j<n) {
//		if (tbl_neighbors[j].type != NB_U) {
//			// copy j'th element to i'th
//			if (i < j) {
//				my_memcpy((uint8_t*)&tbl_neighbors[i], (uint8_t*)&tbl_neighbors[j], sizeof(NeighborInfo));
//			}
//
//			if (tbl_neighbors[j].type == NB_P) {
//				bPrnt = true; // found a parent
//			}
//
//			i++;
//			j++;
//		} else {
//			j++;
//			nNeighbors--;
//			set_log(L_DEL_N);
//		}
//	}
//
//	if (!bPrnt && pParent!=NULL) {
//		// important!!! must update my hop info!
//		myHopInfo.hops = MAX_HOPS;
//		pParent = NULL; // no parent any more
//
//		//set_log(L_DEL_P); // a parent is removed
////debug_info_output("no parent any more...\r\n");
//	}
//	
//	if (nNeighbors <= 0 && n != 0) set_log(L_NO_NB);
}

static NeighborInfo* add_nb(hopinfo_t *info, nb_type type) {
	uint8_t i = 0;

	if (type == NB_U) return NULL;

	for ( ; i<MAX_NEIGHBORS; i++) {
		if (tbl_neighbors[i].type == NB_U) {
			memcpy((uint8_t*)&tbl_neighbors[i].info, (uint8_t*)info, sizeof(hopinfo_t)); // hopinfo_t
			tbl_neighbors[i].type = type; // type
			tbl_neighbors[i].lastcnt = rtc_getcounter(); // timestamp

			if (type == NB_P) {	// add a parent
				// update next hop info
				if (pParent != NULL) update_nb(get_nb_by_id(pParent->info.nodeid), NB_U);
				pParent = &tbl_neighbors[i];
				myHopInfo.hops = pParent->info.hops + 1;
			}
			
			nNeighbors++;
	
//			if (type != NB_U) nNeighbors++;
//			set_log(L_ADD_N);
			return &tbl_neighbors[i];
		}
	}

	return NULL;
}

static void update_nb(NeighborInfo *nb, nb_type type) {
	if (nb->type != type) {	// nb type changed
		if (nb->type == NB_P) {	// lose a parent
			// update next hop info
			pParent = NULL;
			myHopInfo.hops = MAX_HOPS;

			if (type == NB_U) nNeighbors--;	// lose a nb
		} else if (nb->type == NB_C) {
			if (type == NB_U) {	// NB_C --> NB_U
				nNeighbors--; // lose a nb
			} else { // NB_C --> NB_P
				// update next hop info
				if (pParent != NULL) update_nb(get_nb_by_id(pParent->info.nodeid), NB_U);
				pParent = nb;
				myHopInfo.hops = pParent->info.hops + 1;
			}
		} else { // nb->type == NB_U
			if (type == NB_P) {
				// update next hop info
				if (pParent != NULL) update_nb(get_nb_by_id(pParent->info.nodeid), NB_U);
				pParent = nb;
				myHopInfo.hops = pParent->info.hops + 1;
			}
			nNeighbors++;
		}

		// change nb->type
		nb->type = type;
	}

	nb->lastcnt = rtc_getcounter(); // update timestamp
}


//////////////////////////////////
// for debug purpose
// broadcast beacons
//void send_beacon(uint32_t tm) {
//	uint8_t	i;
//	beacon_t beacon;
//	pkt_app_t pkt = {0, NET_RUTE_BCN, 0};
//
//	if (pParent == NULL) return;
//
//	pkt.hdr.nodeid = NODE_ADDRESS;//gConfig.gNodeAddr;
//	beacon._tid = tm;
//	memcpy((uint8_t*)&beacon._info, (uint8_t*)&myHopInfo, sizeof(myHopInfo));
//
//	for (i=0; i<MAX_BEACONS-1; i++) {
//		beacon._seqno = i;
//		my_memcpy((uint8_t*)&pkt.data, (uint8_t*)&beacon, sizeof(beacon_t));
//		node_send_net_command(BROADCAST_ADDRESS, (uint8_t*)&pkt, SIZEOF(beacon_t), 0);
//		delay_ms(INT_BEACONS);
//	}
//	beacon._seqno = i;
//	my_memcpy((uint8_t*)&pkt.data, (uint8_t*)&beacon, sizeof(beacon_t));
//	node_send_net_command(BROADCAST_ADDRESS, (uint8_t*)&pkt, SIZEOF(beacon_t), 0);
//}

//static volatile uint8_t beacons[MAX_NEIGHBORS];
//void deal_rt_beacon(pkt_app_t *pkt) {
//	uint8_t i;
//	for (i=0; i<nNeighbors; i++) {
//		if (tbl_neighbors[i].info.nodeid == pkt->hdr.nodeid) {
//			beacons[i]++;
//			return;
//		}
//	}
//
////	add_nb(&((beacon_t*)(pkt->data))->_info, NB_U); // unknown type neighbor
//	deal_rt_reply(&((beacon_t*)(pkt->data))->_info);
//	beacons[i]++;
//}

//void reset_beacon_array(void) {
//	uint8_t i;
//	for (i=0; i<MAX_NEIGHBORS; i++) {
//		beacons[i] = 0;
//	}
//}

//// do this before a node enters stop mode
//void check_lqi(void) {
//	uint8_t i;
//	for (i=0; i<nNeighbors; i++) {
//		//tbl_neighbors[i].lqi = calc_PDR(beacons[i], MAX_BEACONS);
//		tbl_neighbors[i].lqi = beacons[i];
//	}
//
//	reset_beacon_array();
//}
// eof...
