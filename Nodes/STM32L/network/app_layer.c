#include "app_layer.h"
//#include "stm32_com.h"
//#include "rfbuffer.h"

// buffer for application layer
#define APP_POOL_SIZE POOL_SIZE

static pkt_app_t app_pool_in[APP_POOL_SIZE];
static pkt_app_t *app_pool_start;// = app_pool_in;
static pkt_app_t *app_pool_end;// = app_pool_in + APP_POOL_SIZE - 1;
static pkt_app_t *app_pool_head;// = app_pool_in;
static pkt_app_t *app_pool_tail;// = app_pool_in + APP_POOL_SIZE - 1;
static uint8_t app_pool_items_free;// = APP_POOL_SIZE;
static uint8_t app_pool_items_used;// = 0;
static bool app_pool_overflow_flag;


void init_app_pool(void) {
	reset_app_pool();
}

void reset_app_pool(void) {
	app_pool_start = app_pool_in;
	app_pool_end = &app_pool_in[APP_POOL_SIZE - 1];
	app_pool_head = app_pool_start;
	app_pool_tail = app_pool_end;
	app_pool_items_free = APP_POOL_SIZE;
	app_pool_items_used = 0;
	app_pool_overflow_flag = false;
}

uint8_t get_app_pool_items(void) {
	return app_pool_items_used;
}

pkt_app_t *get_app_pool_handle(void) {
	if (app_pool_items_used == 0) return NULL;

	if (app_pool_tail == app_pool_end) {
		app_pool_tail = app_pool_start;
	} else {
		app_pool_tail++;
	}

	AVR_ENTER_CRITICAL_REGION();
	app_pool_items_free++;
	app_pool_items_used--;
	AVR_LEAVE_CRITICAL_REGION();

	// check for pool overflow
	if (app_pool_overflow_flag) {
		AVR_ENTER_CRITICAL_REGION();
		app_pool_overflow_flag = false;
		AVR_LEAVE_CRITICAL_REGION();
	}

	return app_pool_tail;
}

bool push_app_pool(uint8_t *data, uint8_t len) {
	// pool overflow
	if (app_pool_items_free == 0)	{
//		SoftFault_Handler(1);
		app_pool_overflow_flag = true;
		set_log(L_APP_OVER);
		return false; 
	} else {
		// push into pool head
		memcpy((uint8_t*)app_pool_head, data, len);
//		set_rssi_val(app_pool_head); // set rssi value
	
		// move head pointer to next writable
		if (app_pool_head == app_pool_end)
			app_pool_head = app_pool_start;
		else
			app_pool_head++;
		
		//Turn interrupts off for a short while to protect when status
		//information about the rx_pool is updated.
		//AVR_ENTER_CRITICAL_REGION();
		app_pool_items_free--;
		app_pool_items_used++;
		//AVR_LEAVE_CRITICAL_REGION();

		return true;
	}
}
