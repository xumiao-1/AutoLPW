#ifndef _COMMON_HEADER_H_
#define _COMMON_HEADER_H_


////////////////////////////////////////////////
// keep the same between zgb and zgb_boot
//#define ApplicationAddress(idx)	((idx) == 0 ?  IMG_ADDR_0 : IMG_ADDR_1)

// bootloader:	0x08000000 - 0x08001FFF (8k)
// image0:		0x08002000 - 0x08007FFF	(24k)
// image1:		0x08008000 - 0x0800DFFF	(24k)
// config:		0x0800E000 - 0x0800E7FF (2k)
// reserved:	0x0800E800 - 0x08010000 (6k)
#define BOOT_MAX_SIZE	(0x1000) // 4KB
#define IMG_MAX_SIZE	(0x7400) // 29KB
#define CONFIG_MAX_SIZE	(0x800)	// 2KB
#define BOOT_ADDR	(0x08000000)
#define NODE_INFO_ADDR	(BOOT_ADDR + BOOT_MAX_SIZE) // addr. for node config
#define IMG_ADDR_0	(NODE_INFO_ADDR + CONFIG_MAX_SIZE) // address for image 0
#define IMG_ADDR_1	(IMG_ADDR_0 + IMG_MAX_SIZE)	// address for image 1
//#define NODE_INFO_ADDR	(IMG_ADDR_1 + IMG_MAX_SIZE) // addr. for node config
#define RESERVED_ADDR	(IMG_ADDR_1 + IMG_MAX_SIZE)	// reserved address

#define INT_FLASH_PAGE_SIZE (256) // page size of embedded flash
#define EXT_FLASH_PAGE_SIZE (256) // page size of external flash

#define FLAG_REPROGRAM (0x32F2) // flag that indicates reprogramming
#define FLAG_RESTART (0x32F2) // flag that indicates node on

//#define RG_FLAG		BKP_DR1	// register to indicate if power reset
#define RG_INDX	(RTC_BKP_DR0)	// register for boot index
#define RG_NODE_ADDR	(RTC_BKP_DR1)	// register for node id
#define RG_NODE_TYPE	(RTC_BKP_DR2)	// register for node type
#define RG_NODE_PHAS	(RTC_BKP_DR3)	// register for node phase
#define RG_RESTART	(RTC_BKP_DR4)	// tell if node should be on/off after boot
//#define RG_FLSZ		BKP_DR1	// register to indicate if power reset
//#define RG_CHCK		BKP_DR4	// register for img checksum
////////////////////////////////////////////////

// node type
typedef enum {
	N_WRKSTN = 0,
	N_TMPSNR
} node_type_t;

// node status
typedef enum {
	NODEOFF	= 0,
	STARTUP,
	RUNNING,
	REPROGM
} phase_t;

// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
#define HAL_PORT_NET_LED		( GPIOB )	// PB7
#define HAL_PIN_NET_LED			( GPIO_Pin_7 )
#define HAL_PORT_DATA_LED		( GPIOB )	// PB2
#define HAL_PIN_DATA_LED		( GPIO_Pin_2 )


#endif //_COMMON_HEADER_H_
// eof...
