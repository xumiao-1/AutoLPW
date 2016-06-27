#ifndef _NET_LAYER_H_
#define _NET_LAYER_H_

#include "mydef.h"

//extern hopinfo_t volatile *pParent;
//extern hopinfo_t myHopInfo;

void init_net_pool(void);
void reset_net_pool(void);
bool push_net_pool(uint8_t *appdata, uint8_t len);
pkt_app_t *get_net_pool_handle(void);
uint8_t get_net_pool_items(void);

void init_neighbor_tbl(void);


// deal msg in network layer
void node_net_msg(void);

// ================private func==================
static void update_nb(NeighborInfo*, nb_type);
static NeighborInfo* add_nb(hopinfo_t*, nb_type);

// ================public func==================
hopinfo_t* get_next_hop(void);
void check_nb_tbl(void);
void update_nb_by_id(uint16_t id, nb_type type);
void send_beacon(uint32_t tm);
uint8_t get_nb_num(void);
//NeighborInfo* get_nb_by_idx(uint8_t idx);
void check_lqi(void);
void deal_rt_reply(hopinfo_t *info);
void deal_rt_beacon(pkt_app_t *pkt);

// ================private func=================
void neighbor_discover(void);
void reset_beacon_array(void);

#endif //_NET_LAYER_H_
