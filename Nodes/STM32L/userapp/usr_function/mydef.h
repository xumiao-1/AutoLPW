/**
 * this is a file for learning and testing
 */

#ifndef __MY_LEARNING_H
#define __MY_LEARNING_Hi

#include "include.h"
#include "mystruct.h"


//////////////////////////////////
// macro-definition

// flash memory allocation
#define STM32L
#ifdef STM32L
#define DATA_S_ADDR_EMBEDDED	(0x08010000)
#define DATA_E_ADDR_EMBEDDED	(0x08010000)
#define DATA_S_ADDR_EXTERNAL	(DATA_E_ADDR_EMBEDDED)
#define DATA_E_ADDR_EXTERNAL	(0x08090000)
#else
#endif


// node type and node id
//#define NODE_WRKSTN		(0xAA01)
//#define NODE_TMPSNR		(0xBB01)//(0xBB02)//
//#define NODE_ADDRESS	(NODE_TMPSNR)//(NODE_WRKSTN)//
#define BROADCAST_ADDRESS	(0xFFFF)


// clock frequency
#define SYS_CLK			(32000000)
#define TIM_FREQ_1HZ	(1)
#define TIM_FREQ_1KHZ	(1000)


// RF pkt related
#define FF_FILTER_CNT	(9)	// index: filter
#define FF_TYPE_CNT		(10) // index: type
#define FF_SUBTYPE_CNT	(11) // index: subtype (FUSR_APP or FUSR_NET)
#define FRAME_HEADER_LEN	(12)
#define RF_APP_OFFSET	(FRAME_HEADER_LEN)
#define RF_APP_TAIL_LEN (2)
#define FUSR_APP		(0xB0)	// subtype for application layer
#define FUSR_NET		(0xB1)	// subtype for network layer


// buffer size related
//#define BUFSIZE (256)
#define MAXRECORD (BUFSIZE/SIZEOF(sensor_rpt_data_t))
#define POOL_SIZE		(32)


// node's duty cycle related
#define APP_PERIOD_STARTUP_ON	(1000)
#define APP_PERIOD_STARTUP_OFF	(5000)
#define APP_WKUP_DURA			(10000)
//#define APP_WKUP_INTV			(32000)


#define SIZEOF(structure) ((sizeof(pkt_app_header_t))+(sizeof(structure)))

#define MAX_PDR (0xF)	// max PDR: 100%



//////////////////////////////////
typedef volatile uint32_t vu32;
typedef volatile uint16_t vu16;
typedef volatile uint8_t vu8;


//////////////////////////////////
/********************global variable******************/
extern node_config_t gConfig;
extern uint8_t gRSSI;
extern uint8_t imgbuf[BUFSIZE]; // for reprogram
extern uint16_t fsize; // img size
extern uint16_t fchck; // img checksum
extern uint8_t fpage; // img total pages

#define MAX_DBG_MSG (32)
extern uint8_t dbg_msg[MAX_DBG_MSG];

//////////////////////////////////
// utility functions
void GPIO_config_all_unused(void);
void config_rtc_wkup(uint32_t wkup);
void RTC_WKUP_IRQHandler(void);
void rtc_wkup_irq_init(void);
void usart_irq_init(void);
uint32_t rtc_getcounter(void);

//void node_set_alarm(RTC_TimeTypeDef *pAlarm);
//void node_set_alarm(uint32_t);
void node_sync_clock(RTC_DateTypeDef*, RTC_TimeTypeDef*);
//void node_update_next_alarm(void);
//uint8_t time_add(RTC_TimeTypeDef *val, const RTC_TimeTypeDef *delta);
void node_enter_stop_mode(void);
void node_leave_stop_mode(void);
void SYSCLKConfig_STOP(void);

// int --> date/time
void int2dt(uint32_t ticks, RTC_DateTypeDef *date, RTC_TimeTypeDef *time);
// date/time --> int
uint32_t dt2int(RTC_DateTypeDef date, RTC_TimeTypeDef time);

// serial port output
#define DATA_MSG (0)
#define DBUG_MSG (1)
#define INFO_MSG (2)
//void data_print(uint8_t *msg, uint8_t len);
//void debug_print(uint8_t *msg, uint8_t len);
void my_print(uint8_t type, uint8_t *msg, uint8_t len);

void config_rtc_alarm(void);

// event log operation
void set_log(uint8_t e);
void reset_log(void);
uint8_t get_log(void);
void put_log(sensor_rpt_data_t*);

void save_config(node_config_t*);
void read_config(node_config_t*);

// power on, off & reprogram
void node_power_on(void);
void node_power_off(void);
void node_reprogram(void);
static void node_restart(void);

uint16_t calc_checksum(uint8_t *saddr, uint16_t len);

// for automation
void setup_pin(uint8_t *pt);
void unset_pin(void);
void setup_pin_config(pin_config_t cfg);
void GPIO_RF_config(void);

#endif //__MY_LEARNING_H
