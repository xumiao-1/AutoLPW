#ifndef IIC_DRIVER_H
#define IIC_DRIVER_H


#include <stdint.h>
#include <stdbool.h>
//#include "avr_compiler.h"
//#include "function.h"
#include "stm32l1xx_gpio.h"

#define iic_scl_high()	GPIO_SetBits(GPIOB,GPIO_Pin_10)//(GPIOB->BSRR = GPIO_Pin_10)
#define iic_scl_low()	GPIO_ResetBits(GPIOB,GPIO_Pin_10)//(GPIOB->BRR  = GPIO_Pin_10)
#define iic_sda_high()	GPIO_SetBits(GPIOB,GPIO_Pin_11)//(GPIOB->BSRR = GPIO_Pin_11)
#define iic_sda_low()	GPIO_ResetBits(GPIOB,GPIO_Pin_11)//(GPIOB->BRR  = GPIO_Pin_11)
#define iic_sda_value	(GPIOB->IDR  & GPIO_Pin_11)

void iic_func_enable(void);
uint8_t iic_start(void);
void iic_stop(void);
void iic_ack(void);
void iic_nack(void);
uint8_t iic_chk_ack(void);
uint8_t iic_write_byte(uint8_t data);
uint8_t iic_read_byte(uint8_t *data);
uint8_t iic_dev_write_byte(uint8_t addr, uint8_t data);
uint8_t iic_dev_read_byte(uint8_t addr, uint8_t *data);


#endif

/* EOF */

