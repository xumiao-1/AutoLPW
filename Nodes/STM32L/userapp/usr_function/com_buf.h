#ifndef _COM_BUF_H_
#define _COM_BUF_H_

#include "include.h"
#include "mystruct.h"

/*************public functions*****************/
void reset_com_pool(void);
bool push_com_pool(uint8_t*, uint8_t len);
s_pc_packet_t *get_com_pool_handle(void);
uint8_t get_com_pool_items(void);

#endif // _COM_BUF_H_

// eof...
