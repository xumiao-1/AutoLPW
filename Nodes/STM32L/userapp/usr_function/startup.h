#ifndef _STARTUP_H_
#define _STARTUP_H_

#include "include.h"
#include "mydef.h"

extern uint32_t n_of_record; // # of record in buf
extern uint8_t tmpbuf[BUFSIZE]; // one page size
extern uint32_t lastSync; // last time for sync
extern uint32_t seqno;


//===========public func=========
void startup(void);
void running(void);
void reprogm(void);

//void check_pool(void);
//void reply_schd_req(pkt_app_t*);
//void send_schd_req(uint16_t);
//
//void switch_bootloader(uint8_t, uint16_t, uint16_t);
//uint8_t calc_PDR(uint16_t nSuc, uint16_t nTotal);

//===========private func========
void node_stop(uint8_t *aInCfg);

void enter_node_stop(void);
void leave_node_stop(void);

void set_flag_send(bool);
bool get_flag_send(void);
void set_flag_operation(bool);
bool get_flag_operation(void);
void set_flag_stop(bool);
bool get_flag_stop(void);


typedef enum {
	INTR_UNKNOWN = 0,
	INTR_ALR,
	INTR_KBD
} intr_type_t;

extern volatile intr_type_t eint; // interrupt source



#endif

// eof
