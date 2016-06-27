#ifndef _APP_LAYER_H_
#define _APP_LAYER_H_

#include "mydef.h"


void init_app_pool(void);
void reset_app_pool(void);
bool push_app_pool(uint8_t *appdata, uint8_t len);
pkt_app_t *get_app_pool_handle(void);
uint8_t get_app_pool_items(void);

#endif

// eof
