#include "iic_driver.h"

//#include "stm32l1xx_gpio.h"
#include "user_function.h"


static GPIO_InitTypeDef  GPIO_InitStructure;

void iic_func_enable(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;//GPIO_Speed_40MHz;//Miao
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;//GPIO_Speed_40MHz;//Miao
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

uint8_t iic_start(void)
{
	iic_sda_high();
	delay_us(2);
	iic_scl_high();
	delay_us(5);
	iic_sda_low();
	delay_us(3);
	iic_scl_low();
	delay_us(2);

	return 0;
}

void iic_stop(void)
{
	iic_scl_low();
	delay_us(2);
	iic_sda_low();
	delay_us(2);
	iic_scl_high();
	delay_us(5);
	iic_sda_high();
	delay_us(4);
}

void iic_ack(void)
{
	iic_scl_low();
	delay_us(2);
	iic_sda_low();
	delay_us(2);
	iic_scl_high();
	delay_us(3);
	iic_scl_low();
	delay_us(2);
	iic_sda_high();
	delay_us(2);
}

void iic_nack(void)
{
	iic_scl_low();
	delay_us(2);
	iic_sda_high();
	delay_us(2);
	iic_scl_high();
	delay_us(5);
	iic_scl_low();
	delay_us(2);
}

uint8_t iic_chk_ack(void)
{
	uint8_t iic_flag = 0;
	
	iic_scl_low();
	delay_us(3);
	iic_scl_high();
	delay_us(2);
	if(iic_sda_value == 0)
		iic_flag = 1;
	iic_scl_low();
	delay_us(2);
	
	return iic_flag;
}

uint8_t iic_write_byte(uint8_t data)
{
	uint8_t i = 8;
	for(i=0; i<=7; i++){
		iic_scl_low();
		delay_us(2);
		if(data & 0x80)
			iic_sda_high();
		else iic_sda_low();
		delay_us(2);
		iic_scl_high();
		delay_us(3);
		data <<= 1;
	}
	iic_scl_low();
	delay_us(2);
	
	return 0;
}

uint8_t iic_read_byte(uint8_t *data)
{
	uint8_t i, tmp = 0;
	for(i=0; i<=7; i++){
		iic_scl_low();
		delay_us(3);
		iic_scl_high();
		delay_us(2);
		tmp <<= 1;
		if(iic_sda_value)
			tmp |= 0x01;
		delay_us(1);
	}
	iic_scl_low();	// sp.

	*data = tmp;

	return 0;
}

uint8_t iic_dev_write_byte(uint8_t addr, uint8_t data)
{
	iic_start();
	iic_write_byte(0xa0);
	if(iic_chk_ack() == 0) return 0;
	iic_write_byte(addr);
	if(iic_chk_ack() == 0) return 0;
	iic_write_byte(data);
	if(iic_chk_ack() == 0) return 0;
	iic_stop();

	return 1;
}

uint8_t iic_dev_read_byte(uint8_t addr, uint8_t *data)
{
	iic_start();
	iic_write_byte(0xa0);
	if(iic_chk_ack() == 0) return 0;
	iic_write_byte(addr);
	if(iic_chk_ack() == 0) return 0;
	iic_start();
	iic_write_byte(0xa1);
	if(iic_chk_ack() == 0) return 0;
	iic_read_byte(data);
	iic_nack();
	iic_stop();

	return 1;
}


