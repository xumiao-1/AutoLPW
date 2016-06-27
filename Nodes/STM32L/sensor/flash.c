//#include "include.h"
#include <stdint.h>
#include <stdbool.h>

#include "stm32l1xx_gpio.h"
#include "stm32l1xx_spi.h"

#include "flash.h"
//#include "function.h"
#include "com.h"
#include "user_function.h"
//#include "sys_cfg.h"
//#include "spi_flash.h"

//static uint8_t if_flash_on = true;

uint8_t test_array1[10]={1,2,3,4,5,6,7,8,9,10};
uint8_t test_array2[10];
uint32_t tmp_flash_ID;

/*******************private macro*******************/
// spi flash commands
#define CMD_RD_DEVICE_ID		0x9F // Manufacturer and Device ID Read
#define CMD_RD_STATUS			0xD7 // Status Register Read
#define DUMMY_BYTE				0x55

#define CMD_ERASE_PAGE			0x81 // Page Erase
//#define CMD_ERASE_BLOCK		0x50
//#define CMD_ERASE_SECTOR		0x7C
//#define CMD_ERASE_CHIP		0x

#define	CMD_RD_MEM				0xE8 // Main Memory Page Read

#define CMD_PRG_PAGE_THRU_BUF1	0x82 // Main Memory Page Program Through Buffer 1



/* Select SPI FLASH: Chip Select pin low  */
#define SPI_FLASH_CS_LOW()       GPIO_ResetBits(GPIOB, PIN_CS)
/* Deselect SPI FLASH: Chip Select pin high */
#define SPI_FLASH_CS_HIGH()      GPIO_SetBits(GPIOB, PIN_CS)


void flash_spi_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB ,  ENABLE);//| RCC_APB2Periph_AFIO,
	/* GPIOA, GPIOB and SPI1 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	/* Configure SPI2 pins: SCK and MOSI ---------------------------------------*/
	//GPIO_Pin_13 | GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_12
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);
	//GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_SPI2);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15|GPIO_Pin_14;

	// SPI SCK pin configuration 
	GPIO_InitStructure.GPIO_Pin = PIN_SCK;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//SPI  MOSI pin configuration 
	GPIO_InitStructure.GPIO_Pin =  PIN_SI;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	// SPI MISO pin configuration 
	GPIO_InitStructure.GPIO_Pin = PIN_SO;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	// SPI NSS pin configuration 
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	//GPIO_Init(GPIOB, &GPIO_InitStructure);
  
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15|GPIO_Pin_14;
	//GPIO_Init(GPIOB, &GPIO_InitStructure);

	// RST   SPI NSS
	//GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1|GPIO_Pin_12;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =  PIN_RST | PIN_CS;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//GPIO_ResetBits(GPIOB, GPIO_Pin_12);
	GPIO_SetBits(GPIOB, PIN_RST);
	spi_flash_set(1);
	
	// MISO-PA4
	/*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);*/
	

	/* SPI1 configuration ------------------------------------------------------*/
	SPI_I2S_DeInit(SPI2);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);
	/* Enable SPI1 */
	SPI_Cmd(SPI2, ENABLE);

	// check if page size is 256 bytes
	while (!spi_flash_page_sz256()) {
		spi_flash_format_sz256();
	}
}

void  flash_init(void)
{
	flash_spi_init();
	//spi_flash_init();
}
//*******************************************************************//
void spi_flash_set(uint8_t state) {
	if (state == 0)
		SPI_FLASH_CS_LOW();
	else
		SPI_FLASH_CS_HIGH();
}

void spi_flash_wait(void) {
	uint8_t status = 0;

	// select the flash: set to low votage
	spi_flash_set(0);

	// send "status register read" instruction
	spi_flash_send_byte(CMD_RD_STATUS);

	do { // keep on sending dummy byte to get flash status
		status = spi_flash_send_byte(DUMMY_BYTE);
	} while ( (status & 0x80) == RESET );

	// deselect the flash
	spi_flash_set(1);
}

uint8_t spi_flash_send_byte(uint8_t byte) {
	/* Loop while DR register in not empty */
	while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);
	
	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(FLASH_SPI, byte);
	
	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_RXNE) ==RESET);
	
	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(FLASH_SPI);
}

uint8_t spi_flash_read_byte(void) {
	return spi_flash_send_byte(DUMMY_BYTE);
}

/* convert linear address to data flash address
* For AT45DB041D DataFlash standard page size (264bytes/page),
* lowest 9-bit designates a byte address within a page,
* and higher 11-bit designates a page address
* para:
*	addr - linear address
*	addrByte (out) - DataFlash address
* return: na
*/
void spi_flash_addr(uint32_t addr, uint8_t *addrBytes) {
	// page address
	uint32_t page_addr = addr / SPI_PAGE_SIZE;

	// byte address within a page
	uint32_t byte_addr = addr % SPI_PAGE_SIZE;

#ifdef PAGE_264
	// convert to DataFlash address
	// lowest 9-bit for byte address, and
	// higher 11-bit for page address
	addr = (page_addr << 9) | byte_addr;
#elif defined PAGE_256
	// convert to DataFlash address
	// lowest 8-bit for byte address, and
	// higher 11-bit for page address
	addr = (page_addr << 8) | byte_addr;
#endif

	addrBytes[0] = (uint8_t)(addr >> 16);
	addrBytes[1] = (uint8_t)(addr >> 8);
	addrBytes[2] = (uint8_t)(addr);
} 


/* get spi device id
* para:		na
* return:	device id
*/
uint32_t spi_flash_get_id(void) {
	uint8_t bytes[4];

	
	/* Select the FLASH: Chip Select low */
	spi_flash_set(0);
	
	/* send "read device id" command */
	spi_flash_send_byte(CMD_RD_DEVICE_ID);//SPI_FLASH_SendByte(0X9F);

	bytes[0] = spi_flash_read_byte(); // read a byte from flash
	bytes[1] = spi_flash_read_byte();
	bytes[2] = spi_flash_read_byte();
	bytes[3] = spi_flash_read_byte();

	/* Deselect the FLASH: Chip Select high */
	spi_flash_set(1); 

	return ((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3]);
}

/* erase a page
* para:	
*	addr - linear address
* return: na
*/
void spi_flash_erase_page(uint32_t addr) {

	/* convert address */
	uint8_t addrBytes[3]; // DataFlash address
	spi_flash_addr(addr, addrBytes);

	/* Select the FLASH: Chip Select low */
	spi_flash_set(0);

	/* Send "Erase page" instruction */
	spi_flash_send_byte(CMD_ERASE_PAGE);//SPI_FLASH_SendByte(PAGE_ERASE);

	/* Send address to erase */
	spi_flash_send_byte(addrBytes[0]);
	spi_flash_send_byte(addrBytes[1]);
	spi_flash_send_byte(addrBytes[2]);

	/* Deselect the FLASH: Chip Select high */
	spi_flash_set(1);//SPI_FLASH_CS_HIGH();

	/* wait for writing task end */
	spi_flash_wait();
}
/* read data from flash
* para:
*	addr - linear address
*	buf (out) - buffer to store data read from flash
*	sz - length of data to be read from flash
* return:
*	how many bytes actually been read from flash
*/
uint32_t spi_flash_read_data(uint32_t addr, uint8_t *buf, uint32_t sz) {
	uint8_t addrBytes[3]; // DataFlash address
//	uint32_t len = 0;	// length of data been read
	uint32_t i; // length of data been read

	// convert to DataFlash address from linear address
	spi_flash_addr(addr, addrBytes);

	// select flash
	spi_flash_set(0);

	// send "Main Memory Page Read" command
	spi_flash_send_byte(CMD_RD_MEM);
	// send address
	for (i=0; i<3; i++) 
		spi_flash_send_byte(addrBytes[i]);
	// send 4 bytes dont care
	for (i=0; i<4; i++)
		spi_flash_send_byte(DUMMY_BYTE);

	// now read data from flash
	for (i=0; i< sz; i++) {
		*buf++ = spi_flash_read_byte();
	}

	// deselect flash (put into standby mode)
	spi_flash_set(1);

	return i;
}
/* write data to flash through buffer1
* para:
*	addr - linear address
*	buf - data buffer that to be written into flash
*	sz - length of data to be written
* return:
*	length of data that actually been written
*/
uint32_t spi_flash_write_data(uint32_t addr, uint8_t *buf, uint32_t sz) {
	uint32_t i;
	uint8_t addrBytes[3];

	// convert address
	spi_flash_addr(addr, addrBytes);

	// select flash
	spi_flash_set(0);

	// send "Main Memory Page Program Through Buffer 1"	command
	spi_flash_send_byte(CMD_PRG_PAGE_THRU_BUF1);
	// send address
	for (i=0; i<3; i++)
		spi_flash_send_byte(addrBytes[i]);
	
	// write data to buffer1
	for (i=0; i<sz; i++) {
		spi_flash_send_byte(*buf++);
	}

	// deselect flash (will start to program flash from buffer1)
	spi_flash_set(1);
	//delay_ms(5);
	// wait for task finish
	spi_flash_wait();

	return i;
}

// Miao
void set_spi_flash_state(uint8_t state) {
	/* Select the FLASH: Chip Select low */
	spi_flash_set(0);
	
	/* send command */
	if (SPI_FLASH_OFF == state) { // OFF
		spi_flash_send_byte(SLEEPDOWN);
	} else { // ON
		spi_flash_send_byte(WAKEUP);
	}

	/* Deselect the FLASH: Chip Select high */
	spi_flash_set(1); 
}


// check if page size is 256 bytes
bool spi_flash_page_sz256(void) {
	uint8_t status = 0;

	// select the flash: set to low votage
	spi_flash_set(0);

	// send "status register read" instruction
	spi_flash_send_byte(CMD_RD_STATUS);

	status = spi_flash_send_byte(DUMMY_BYTE);

	// deselect the flash
	spi_flash_set(1);

	return (status & 0x01) == 0x01;
}

// change page size to 256 bytes
void spi_flash_format_sz256(void) {
	uint8_t i;
	uint8_t cmd[4] = {0x3D, 0x2A, 0x80, 0xA6}; // opcode

	// select the flash
	spi_flash_set(0);

	// send opcode
	for (i=0; i<4; i++) {
		spi_flash_send_byte(cmd[i]);
	}

	// deselect the flash
	spi_flash_set(1);

	// wait for the task finish
	spi_flash_wait();
}
//************************************************************************//
//bool  node_test_flash(void )
//{
//	bool test_flag=true;
//	uint8_t count;
//	uint8_t array_sum=0;
//
//	flash_init();
//	
//	tmp_flash_ID=0;
//	tmp_flash_ID=spi_flash_get_id();
//	
//	spi_flash_write_data(0x00000800,test_array1,10);
//	
//	spi_flash_read_data(0x00000800,test_array2,10);
//
//	for(count=0;count<10;count++)
//	{
//		array_sum+=test_array2[count];
//	}
//
//	if(array_sum==55)
//	{
//		test_flag=true;
//	}
//	else{
//		test_flag=false;
//	}  
//	return test_flag;
//	
//}

