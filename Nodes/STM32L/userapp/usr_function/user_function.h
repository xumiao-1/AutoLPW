#ifndef USER_FUNCTION_H
#define USER_FUNCTION_H

//#include "avr_compiler.h"
#include <stdint.h>
#include <stdbool.h>

#define PROG_START 0x0000
#define BOOT_START 0xF000

#define U8TOU16(u8h,u8l) ((uint16_t)((u8h)<<8)|(u8l))

void reset(void);
uint16_t crc_ccitt_update(uint16_t crc, uint8_t data);
uint8_t XorGeneration(uint8_t *data, const uint8_t len);
void bufcpy(uint8_t *dest, uint8_t *src, uint16_t len);
void delay_us(uint16_t us);
void delay_ms(uint16_t ms);
void SysTick_Configuration(void);
void DecrementTimingDelay(void);

#endif

