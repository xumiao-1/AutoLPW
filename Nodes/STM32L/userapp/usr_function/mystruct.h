#ifndef _MYSTRUCT_H_
#define _MYSTRUCT_H_

#include <stdint.h>
#include "common_def.h"

#define FULL_VER
#define AUTOMATION
#define CONFIG_LEN (100)
#define SNR_NODE (0xBB01)
#define PRX_NODE (0xAA01)
#define NODE_ADDR (SNR_NODE)//(PRX_NODE)//

#define BUFSIZE (256)
#define PKT_IMG_SIZE	(64)
#define NUM_CHUNKS 		(BUFSIZE / PKT_IMG_SIZE)
#define NUM_PAGES		(1 + (IMG_MAX_SIZE - 1) / BUFSIZE) //Ceiling of (IMG_MAX_SIZE / BUFSIZE)


/********************************************
 * enum type
 ********************************************/

//// node status
//typedef enum {
//	STARTUP = 0,
//	RUNNING
//} phase_t;

// RF cmd type
typedef enum {
	RF_UNKNOWN = 0,

	// wireless reprogram
	RF_PROG_INF, // img info
	RF_PROG_DAT, // img data

	// app layer
	APP_SCHD_REQ,
	APP_SCHD_REP,
	APP_DATA_TMP,
	APP_INTV_CMD,
	//APP_NODE_OFF,
	APP_NODE_RESTART, // restart msg

	// net layer
	NET_RUTE_REQ,
	NET_RUTE_REP,
	NET_RUTE_BCN,

	// wireless reprogram
	RF_PROG_NAK, // img NAK
	RF_PROG_INIT, //initial img info
	RF_INIT_NAK,  //img info NAK

	// misc
	MISC_MSG = 127
} cmd_t;

// Serial cmd type
typedef enum {
	S_UNKNOWN = 0,

	S_CMD_SYNC, // sync time with pc
	S_CMD_UPLD,	// upload data to pc
	S_CMD_DUMP, // dump memory to pc
	S_CMD_INTV,	// change app interval
	S_CMD_UPDT,	// update image
	S_CMD_SHUT,	// shutdown node
	S_CMD_IMGS,
	S_CMD_IMGE,
	S_CMD_IMGD,  // image detail

	S_CMD_RESTART_REPROG // restart for reprogramming
} s_cmd_t;

// neighbor type
typedef enum {
	NB_U = 0,
	NB_P,
	NB_C
} nb_type;

// event log type
// 8-bit, 
// so in total 8 event types
typedef enum {
	L_ADD_N = 0x01,	// add a nb (not using)
	L_DEL_N = 0x02,	// del a nb (not using)
	L_REP_P = 0x04,	// replace parent node
	L_SYNC 	= 0x08,	// time synced
	L_NO_NB	= 0x10, // no nbs (not using)
	L_APP_OVER = 0x20 // app_pool overflow
//	L_IN_WT,
//	L_OUT_WT
} log_t;

typedef enum {
	// Mode
#ifdef FULL_VER
	CFG_GP_PP_0 = 0, 
	CFG_GP_PP_1, 
	CFG_GP_PP_PU_0,
	CFG_GP_PP_PU_1, 
	CFG_GP_PP_PD_0, 
	CFG_GP_PP_PD_1, 
	CFG_GP_OD_0, 
	CFG_GP_OD_1,
	CFG_GP_OD_PU_0, 
	CFG_GP_OD_PU_1, 
	CFG_GP_OD_PD_0, 
	CFG_GP_OD_PD_1, 
	CFG_IN_FLOAT,
	CFG_IN_PU, 
	CFG_IN_PD, 
	CFG_AN
#else
	CFG_GP_PP_0 = 0, 
	CFG_GP_PP_1, 
	CFG_GP_PP_PU_0,
	CFG_GP_PP_PD_1, 
	CFG_GP_OD_1,
	CFG_GP_OD_PU_1, 
	CFG_GP_OD_PD_1,
	CFG_AN
#endif
} config_typedef;


/********************************************
 * struct type
 ********************************************/

#pragma pack (1)

//==============begin================================
// structure for pin config (16bit)
// 
typedef struct {
	uint8_t grp:4; // grp#: 0(pa), 1(pb)...
	uint8_t pin:4; // pin#: 0..15
	uint8_t res:3;
	uint8_t config:5; // config
} pin_config_t;


// define the node config info
typedef struct {
	uint16_t gNodeAddr;
	uint8_t gNodeType;
	uint8_t gPhase;
	
	uint8_t gPDR; // PDR (4-bit): 0x0 ~ 0xF
	uint8_t	gRand;
	//bool		bScheduled;

	uint32_t	nextAlarm;//RTC_TimeTypeDef nextAlarm;//
//	uint16_t	APP_PERIOD_RUNNING_OFF;
	uint16_t	APP_WKUP_INTV;	// wakeup interval
	//uint16_t	APP_WKUP_DURA;	// wakeup duration
} node_config_t;
//==============end==================================

//==============begin================================
// the following data structures are used by
// other structures.

// this structure defines a node's duty cycle
typedef struct {
	uint16_t intv;	// node's wakeup interval
	uint16_t dura;	// node's wakeup duration
} wkup_info_t;

// this structure defines the img size and checksum,
// it is used for wireless reprogramming
typedef struct {
	uint16_t fsize;	// img size
	uint16_t fchck;	// img checksum
} img_info_t;

// this structure defines the shutdown info
typedef struct {
	uint16_t nodeid; // if turn off all nodes, make it 0x0.
	uint16_t reserved;
} shut_info_t;

// this structure defines link quality
typedef struct {
	uint16_t nodeid;
	unsigned relation: 2; // U, P, C
	unsigned pdr: 6;
} link_lqi_t;
//===============end===============================

//===============begin===============================
// the following structures define the pkt format,
// including the serial pkt & RF pkt
// understanding of these structures is a "MUST"

// serial packet format
typedef enum {
	// commands from nodes
	PC_CMD_ACT = 0,
	SNR_CMD_RDY = 1,
	PC_CMD_CFG,
	SNR_CMD_LPW_BGN,
	SNR_CMD_LPW_END,

	// commands from proxy nodes
	//PRX_CMD_CONFIG = 10
	AUTO_CMD = 10
} s_sensor_cmd_t;

typedef struct {
	uint8_t cmd;
	uint8_t res;
} s_sensor_pkt_t;

typedef struct {
	uint8_t cmd; // cmd type
	union {
		uint32_t tval; // invalid if _cmd == 0x01
		wkup_info_t wkup_info;// wkup setting
		img_info_t img_info ; // img property
		shut_info_t shut_info; // shutdown info
	} data;
} s_pc_packet_t;

typedef struct {
	uint8_t cmd; // cmd type (always be 
							// S_CMD_IMGD for image details)
	uint8_t pg_no;
	uint8_t seq_no;
	uint8_t img[PKT_IMG_SIZE];
} s_img_packet_t;

//Image buffer information + buffer itself
typedef struct {
	uint8_t recv_seq_no[NUM_CHUNKS];
	uint8_t page_no;
	//uint8_t max_pages;
	uint8_t expec_seq_no;
	uint32_t last_out_of_sync;
	//uint16_t fsize;
	//uint16_t fchck;
	uint8_t page[BUFSIZE];
} img_buffer_t;

typedef struct {
	uint32_t sn; // serial # to avoid broadcast storm
	uint8_t page_no;	// page no.
	uint8_t seq_no;		// seq no.
	uint8_t fin;
	uint8_t img[PKT_IMG_SIZE];	// size that contained in each pkt
} program_data_t;

typedef struct {
	uint8_t max_pages; // total img pages: max_pages = ceiling of (fsize/BUFSIZE)
	//possible other information
	// add by Miao Jun@2013
	uint16_t fsize;	// total img size
	uint16_t fchck;	// img checksum, for integrity check
	uint32_t sn; // serial #, to avoid broadcast storm
} img_init_t;

// RF packet format: pkt header
typedef struct {
	uint16_t nodeid;
	uint8_t cmd;
	uint8_t rssi;
} pkt_app_header_t;

// RF packet format: pkt header + pkt payload
#define MAX_APPDATA_LENGTH	(96)
typedef struct {
	pkt_app_header_t hdr;
	uint8_t data[MAX_APPDATA_LENGTH];
} pkt_app_t;

// RF packet format: pkt payload
// this payload is used to transmit the sampled data.
// It contains the temperature, water level etc.
//typedef struct {
//	// == first 12 bytes
//	uint32_t _tid;		   // sample time
//	uint16_t _tmpr;		   // temperature, lowest 12bits (the highest 4 bits are reserved for PDR)
//	uint8_t _event;			// event log (log_t)
//	unsigned _hops: 2;	// no forwarding: 0; otherwise: 1
//	unsigned _water_e: 1;	// water level detector by electrode. 1 - under water; 0 - above water.
//	unsigned _water_c: 1;	// water level detector by capacitor. 1 - under water; 0 - above water.
//	unsigned _reserved: 4;
//	uint32_t trecv;			// received time at host
//
//	// == second 16 bytes
//	uint16_t voltage_e;		// reading from electrode
//	uint16_t voltage_c;		// reading from capacitor
//	uint16_t voltage_bat;		// battery voltage
//	uint8_t unused111[4];
//	uint16_t nodes_hopped[3];// intermediate nodes
//
//	// == last 32 bytes
//	link_lqi_t _lqi[8];		// 3 * 8 = 24bytes
////	uint8_t unused2[4];
//	uint32_t _bufno;		// buffered packet #
//	uint32_t _seqno;		// sequence no.
//} sensor_rpt_data_t;
typedef struct {
	// == first 12 bytes
	uint32_t _tid;		   // sample time
	uint16_t _tmpr;		   // temperature, lowest 12bits (the highest 4 bits are reserved for PDR)
	uint8_t _event;			// event log (log_t)
	unsigned _hops: 2;	// no forwarding: 0; otherwise: 1
	unsigned _water_e: 1;	// water level detector by electrode. 1 - under water; 0 - above water.
	unsigned _water_c: 1;	// water level detector by capacitor. 1 - under water; 0 - above water.
	unsigned _pdr: 4;
	uint32_t trecv;			// received time at host

	// == second 16 bytes
	//uint16_t voltage_e;		// reading from electrode
	uint16_t voltage_c;		// reading from capacitor
	uint16_t voltage_bat;		// battery voltage
	uint16_t nodes_hopped[3];// intermediate nodes
	uint16_t _bufno;		// buffered packet #
	uint32_t _seqno;		// sequence no.
} sensor_rpt_data_t;

// sink's report data (in fact, the same with the sensor node's)
typedef sensor_rpt_data_t host_rpt_data_t;

// RF packet format: pkt payload
// this payload is used for time synchronization
typedef struct {
//	uint32_t curtime; // current local time
//	uint32_t nxtalrm; // next alarm
//	uint16_t intv;	  // interval
//	uint16_t dura;	  // duration
	RTC_DateTypeDef date; // current date
	RTC_TimeTypeDef time; // current time
	uint32_t nxtalrm;//RTC_TimeTypeDef nxtalrm; // next alarm
	uint16_t intv;	  // interval
	uint16_t dura;	  // duration
} sync_data_t;
typedef struct {
	uint16_t intv;	  // interval
	uint16_t dura;	  // duration
} sync_data1_t;

// RF packet format: pkt payload
// restart msg thru. rf
#define SUB_RESTART_OFF 0
#define SUB_RESTART_ON 1
#define SUB_RESTART_REPROG 2
//typedef struct {
//	uint16_t seqno;	// seqno, in case broadcasting storm
//	uint16_t nodeid;
//	uint8_t subcmd; // off, restart, restart with reprog
//	uint8_t reserved[3]; // reserved
//} restart_msg_t;
// this payload is used for turn off sensor nodes
typedef struct {
	//uint16_t seqno;	// seqno, in case broadcasting storm
	uint8_t seqno;	// seqno, in case broadcasting storm
	uint8_t subcmd; // off, restart, restart with reprog
	uint16_t nodeid;
} shut_data_t;


//typedef struct {
//	uint16_t seqno;
//	uint16_t intv;
//} intv_data_t;
//=============end=================================


// a node's hop info 
typedef struct {
	uint16_t nodeid;
	uint8_t hops; // # of hops to sink node
} hopinfo_t;

// a node's neighbor
typedef struct {
	hopinfo_t info;
	uint8_t type; // relation with local node: P(arent), C(hild), U(nknown)
	uint8_t lqi;
	uint32_t lastcnt; // last timestamp that is updated
} NeighborInfo;

#pragma pack ()


#endif // _MYSTRUCT_H_

// eof...
