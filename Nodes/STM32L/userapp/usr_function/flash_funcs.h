/**
 * This file deals with built-in flash memory of STM32
 *
 * created by: Thomas
 * modified by: Miao@Jan. 2012
 */
#ifndef FLASH_FUNCS_H
#define FLASH_FUNCS_H

#include <time.h>
#include "include.h"




//Function Prototypes
// public functions...
void write_flash(uint8_t *buffer);
void read_flash(uint32_t saddr, uint32_t len, uint8_t *pval);
uint32_t get_flash_head(void);
uint32_t get_flash_tail(void);
uint32_t get_page_num(void);
bool is_flash_empty(void);
void erase_page(void);

// add by Miao @Apr. 2013
// universe API	to write flash,
// including embedded/external flash
void write_flash_with_addr_len(uint32_t, uint8_t*, uint16_t);
void write_flash_with_addr(uint32_t, uint8_t*);


// private functions...

// add by Miao
static void reset_flash(void);
static void write_embedded_flash_with_addr_len(uint32_t, uint8_t*, uint16_t);
static void write_embedded_flash_with_addr(uint32_t, uint8_t*);



#endif
// eof...
