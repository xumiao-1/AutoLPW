#include "com_buf.h"

#define COM_POOL_SIZE 16

static s_pc_packet_t com_pool[COM_POOL_SIZE];
static s_pc_packet_t *com_pool_start = com_pool;
static s_pc_packet_t *com_pool_end = com_pool + COM_POOL_SIZE - 1;
static s_pc_packet_t *com_pool_head = com_pool;
static s_pc_packet_t *com_pool_tail = com_pool + COM_POOL_SIZE - 1;
static volatile uint8_t com_pool_free = COM_POOL_SIZE;
static volatile uint8_t com_pool_used = 0;

void reset_com_pool(void) {
	com_pool_head = com_pool_start;
	com_pool_tail = com_pool_end;
	com_pool_free = COM_POOL_SIZE;
	com_pool_used = 0;
}

uint8_t get_com_pool_items(void) {
	return com_pool_used;
}

s_pc_packet_t *get_com_pool_handle(void) {
	if (com_pool_used == 0) return NULL;

	if (com_pool_tail == com_pool_end) {
		com_pool_tail = com_pool_start;
	} else {
		com_pool_tail++;
	}

	com_pool_free++;
	com_pool_used--;

	return com_pool_tail;
}

bool push_com_pool(uint8_t *appdata, uint8_t len) {
	// pool overflow
	if (com_pool_free == 0)	{
		return false; 
	} else {
		// push into pool head
		uint8_t *pbyte = (uint8_t*)com_pool_head;
		do {
			*pbyte++ = *appdata++;
		} while (--len > 0);
	
		// move head pointer to next writable
		if (com_pool_head == com_pool_end)
			com_pool_head = com_pool_start;
		else
			com_pool_head++;
	
		com_pool_free--;
		com_pool_used++;
	
		return true;
	}
}

// eof...
