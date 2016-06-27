#ifndef FLASH_H
#define FLASH_H

//#include "avr_compiler.h"
//#include <avr/io.h>

// Miao
#define SPI_FLASH_OFF	0
#define SPI_FLASH_ON	1

//#define _NOP() asm volatile ("nop\n\t" ::)
//
/*
#define SPO_HIGH         PORTD |= 0X08
#define SPO_LOW          PORTD &= ~0x08
#define SPI_CS_HIGH      PORTD |= 0x80
#define SPI_CS_LOW       PORTD &= ~0x80
#define SCK_HIGH         PORTD |= 0x20
#define SCK_LOW          PORTD &= ~0x20
#define SPI_IN           PIND & 0x04
*/
//#define PROGRAMMEMORYBUFFER1 0x82
#define MAINMEMORYREAD       0x52
#define READID               0x9F

#define PIN_CS		GPIO_Pin_12
#define PIN_SCK		GPIO_Pin_13
#define PIN_SO		GPIO_Pin_14
#define PIN_SI		GPIO_Pin_15
#define PIN_RST		GPIO_Pin_1
#define FLASH_SPI	SPI2
//#define RCC_APB1Periph_FLASH_SPI	RCC_APB1Periph_SPI2

// define which page size is used (256 or 264)
#define PAGE_256

#ifdef PAGE_264
	#define SPI_PAGE_SIZE 264	// standand DataFlash page size
#elif defined PAGE_256
	#define SPI_PAGE_SIZE 256	// Power-of-2 DataFlash page size
#endif

#define FLASH_041D 0x1c
#define FLASH_081D 0x24


//AT45DB161D command
#define SLEEPDOWN                 0XB9
#define WAKEUP                    0XAB
#define MAINMEMORYPAGEREAD        0XD2
#define CONTINUOUSARRAYREAD       0XE8
#define READBUFFER1               0XD1
#define READBUFFER2               0XD3
#define WRITEBUFFER1              0X84
#define WRITEBUFFER2              0X87
#define READSTATUS                0XD7
#define CONTINOUSREAD             0XE8
#define PROGRAMMEMORYBUFFER1      0X82
#define PROGRAMMEMORYBUFFER2      0X85
#define READMEMORY                0XD2
#define BUFFER1TOMEMORYERASE      0X83
#define BUFFER2TOMEMORYERASE      0X86
#define BUFFER1TOMEMORYNOERASE    0X88
#define BUFFER2TOMEMORYNOERASE    0X89
#define MEMORYTOBUFFER1           0X53
#define MEMORYTOBUFFER2           0X55
#define MEMORYBUFFER1COMPARE      0X60
#define MEMORYBUFFER2COMPARE      0X61
#define BLOCKERASE 	              0X50
#define PAGEERASE		          0x81

//bool flash_valid;

//bool test_flash(void);
void flash_spi_init(void);


void spi_flash_set(uint8_t state);
uint8_t spi_flash_send_byte(uint8_t byte);
uint8_t spi_flash_read_byte(void);
void spi_flash_wait(void);

void spi_flash_addr(uint32_t addr, uint8_t *addrBytes);

uint32_t spi_flash_get_id(void);
void spi_flash_erase_page(uint32_t addr);
uint32_t spi_flash_read_data(uint32_t addr, uint8_t *buf, uint32_t sz);



//void reset_flash(void);
void  flash_init(void);
//uint8_t transByte(uint8_t data);

void set_spi_flash_state(uint8_t state);


// check if page size is 256 bytes
bool spi_flash_page_sz256(void);

// change page size to 256 bytes
void spi_flash_format_sz256(void);
//bool  node_test_flash(void );

// ==========================public===============================
uint32_t spi_flash_write_data(uint32_t addr, uint8_t *buf, uint32_t sz);

#endif

