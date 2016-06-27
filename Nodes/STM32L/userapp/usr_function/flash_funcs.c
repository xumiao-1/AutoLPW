/**
 * created by: Thomas
 * modified by: Miao@Jan. 2012
 */
#include "flash_funcs.h"
//#include "platform.h" cannot locate file
//#include "avr_hal.h"
#include "tat.h"
//#include "rfbuffer.h"
#include "function.h"
//#include "net_common.h"
//#include "sys_cfg.h"
#include "sensor_tmp102.h"
#include "stm32l1xx_it.h"
#include "flash.h"
#include "mydef.h"

#define DATA_S_ADDR	(DATA_S_ADDR_EMBEDDED)
#define DATA_E_ADDR (DATA_E_ADDR_EXTERNAL)
//#define FLASH_PAGE_SIZE ((u16)0x800)
#define MAXFLASHSIZE (DATA_E_ADDR - DATA_S_ADDR)
#define MAXPAGENUM (MAXFLASHSIZE/BUFSIZE)



// add by Miao
// we use the flash in a circular way
// flash_head: the first page that will be read from
// flash_tail: the next page that will be written to
static uint32_t n_of_page = 0; // # of page used
static uint32_t flash_head = DATA_S_ADDR;
static uint32_t flash_tail = DATA_S_ADDR;

static volatile FLASH_Status FLASHStatus;

// modified by Miao to support external flash, May 2012
/**
* This function writes data to flash memory.
* The function also changes flash_head, flash_tail when
* it quits.
*/
void write_flash(uint8_t *buffer) {
	if (flash_tail < DATA_E_ADDR_EMBEDDED) { // write to embedded flash
		write_embedded_flash_with_addr(flash_tail, buffer);
	} else { // write to external flash
		spi_flash_write_data(flash_tail-DATA_E_ADDR_EMBEDDED, buffer, BUFSIZE);
	}

	// update flash_head, flash_tail & n_of_page
	flash_tail += BUFSIZE;
	if (flash_tail >= DATA_E_ADDR) flash_tail = DATA_S_ADDR; // use the flash in a circular way

	if (flash_tail == flash_head) { // move flash_head if necessary
		flash_head += BUFSIZE;
		flash_head = flash_head >= DATA_E_ADDR ? DATA_S_ADDR : flash_head;
	}

	n_of_page++; // increase page # by 1
	if (n_of_page >= MAXPAGENUM) n_of_page = MAXPAGENUM;
} 

// modified by Miao to support external flash, May 2012
/**
* This function reads data from flash memory.
* this function should be treated carefully!!!!!!!
* never read from both flashes at the same time!!!
*/
void read_flash(uint32_t saddr, uint32_t len, uint8_t *pval) {
	if (saddr < DATA_E_ADDR_EMBEDDED) { // read from embedded flash
		uint8_t *paddr = (uint8_t*)((vu32*)saddr); //pointer to start adress
		uint8_t *peaddr = (uint8_t*)((vu32*)(saddr+len)); //pointer to end address

		//while start address is less then end address
		while(paddr < peaddr){
			*pval = *paddr; //value of pval is value at beginning adress
			paddr++; //increase beginning address by 8 bytes
			pval++; //increase pval by 8 bytes
		}
	} else { // read from external flash
		spi_flash_read_data(saddr-DATA_E_ADDR_EMBEDDED, pval, len);
	}
}

// add by Miao
uint32_t get_flash_head(void) { return flash_head; }
uint32_t get_flash_tail(void) { return flash_tail; }
uint32_t get_page_num(void) { return n_of_page; }
//void reset_flash(void) {
void init_data_storage(void) {
	n_of_page = 0;
	flash_head = flash_tail = DATA_S_ADDR;
}

bool is_flash_empty(void) {
	return n_of_page == 0;
}

void erase_page(void) {
	flash_head += BUFSIZE;
	flash_head = flash_head >= DATA_E_ADDR ? DATA_S_ADDR : flash_head;
	n_of_page--;
}

// write to a specified page in flash
// page_addr: the page address that write to
void write_embedded_flash_with_addr_len(uint32_t page_addr, uint8_t *buf, uint16_t len) {
	uint32_t saddr = page_addr;			// start address for the page to write
	uint32_t eaddr = saddr+len;	// end addrees for that page
	uint32_t *pbuf = (uint32_t*)buf;

	// Unlock the Flash Program Erase controller
	FLASH_Unlock();

	// Clear All pending flags 
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR);

	// Erase the FLASH pages 
	FLASHStatus = FLASH_ErasePage(saddr);
	
	// write buf to that page
	while((saddr < eaddr) && (FLASHStatus == FLASH_COMPLETE)) {
		FLASHStatus = FLASH_FastProgramWord(saddr, *(pbuf));	//problem
		saddr = saddr + 4;
		pbuf++;
	}

	if (FLASHStatus != FLASH_COMPLETE) {
		HardFault_Handler();//enter_unexpected();  do something here;
	}


	// Lock flash
	FLASH_Lock();
}
// write to a specified page in flash
// page_addr: the page address that write to
void write_embedded_flash_with_addr(uint32_t page_addr, uint8_t *buf) {
	write_embedded_flash_with_addr_len(page_addr, buf, BUFSIZE);
}

// add by Miao @Apr. 2013
// universe API	to write flash,
// including embedded/external flash
void write_flash_with_addr_len(uint32_t page_addr, uint8_t *buf, uint16_t len) {
	if (page_addr < DATA_E_ADDR_EMBEDDED) { // write to embedded flash
		write_embedded_flash_with_addr_len(page_addr, buf, len);
	} else { // write to external flash
		spi_flash_write_data(page_addr-DATA_E_ADDR_EMBEDDED, buf, len);
	}
}

void write_flash_with_addr(uint32_t page_addr, uint8_t *buf) {
	write_flash_with_addr_len(page_addr, buf, BUFSIZE);
}

// eof...

