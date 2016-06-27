#include "sensor_tmp102.h"
#include "com.h"
//#include "iic_driver.h"
#include "stm32l1xx_it.h"


static tmp102_address_t tmp102_address;
static tmp102_config_t tmp102_config;
static tmp102_write_t write_config;
static tmp102_read_t read_value;
static tmp102_write_t *p_config = &write_config;
static tmp102_read_t *p_value = &read_value;


//static uint16_t tmp16 = 0;

void tmp102_init_config(void)
{
	iic_func_enable();
	
	tmp102_address.addr = ADDR_GND;
	tmp102_address.rwflag = RWFLAG_W;

	tmp102_config.SD = SD_ENABLE; // SD_DISABLE; // Miao
	tmp102_config.TM = COMPARATOR_MODE;
	tmp102_config.POL = POL_DISABLE;
	tmp102_config.FQ = CONSECUTIVE_FAULTS_1;
	tmp102_config.R	= 0;	// converter resolution (read only)
	tmp102_config.OS = ONE_SHOT_ENABLE; // ONE_SHOT_DISABLE; // Miao
	tmp102_config.RES = 0;
	tmp102_config.EM = EXTENDED_MODE;	//NORMAL_MODE;	
	tmp102_config.AL = 0;	// (read only)
	tmp102_config.CR = CR_4_HZ;	// conversion rate
	
	p_config->addr = *((uint8_t *)&tmp102_address);
	p_config->cfg = *((uint16_t *)&tmp102_config);
	//p_config->addr = 0x90;
	//p_config->cfg = 0x9000;	//0xa060;
	p_config->preg = POINTER_CFG_REG;
	
	if(tmp102_write_config(p_config) == 0){
		HardFault_Handler();
	}
	//delay_ms(50);
	//tmp102_read_config_reg(ADDR_GND, &tmp16);
}

uint8_t tmp102_write_config(tmp102_write_t *cnfg)
{
	uint8_t cfg_h, cfg_l;
	uint16_t cfg_w = (uint16_t)(cnfg->cfg);
	cfg_l = (uint8_t)cfg_w;
	cfg_h = (uint8_t)(cfg_w >> 8);
	
	iic_start();
	iic_write_byte((uint8_t)(cnfg->addr) & RWMASK_W);
	if(iic_chk_ack() == 0) return 0;
	iic_write_byte((uint8_t)(cnfg->preg));
	if(iic_chk_ack() == 0) return 0;
	iic_write_byte(cfg_l);
	if(iic_chk_ack() == 0) return 0;
	iic_write_byte(cfg_h);
	if(iic_chk_ack() == 0) return 0;
	iic_stop();

	return 1;
}

uint8_t tmp102_read_word_reg(tmp102_read_t *cnfg)
{
	
	iic_start();
	iic_write_byte((uint8_t)(cnfg->addr) & RWMASK_W);
	if(iic_chk_ack() == 0) return 0;
	iic_write_byte((uint8_t)(cnfg->preg));
	if(iic_chk_ack() == 0) return 0;
	//iic_stop();
	//delay_us(20);
	iic_start();
	iic_write_byte((uint8_t)(cnfg->addr) | RWMASK_R);
	if(iic_chk_ack() == 0) return 0;
	iic_read_byte((uint8_t *)(cnfg->value) + 1);
	iic_ack();
	iic_read_byte((uint8_t *)(cnfg->value));
	iic_ack();	//iic_nack();
	iic_stop();

	return 1;
}

uint8_t tmp102_read_config_reg(uint8_t addr, uint16_t *value)
{
	p_value->addr = addr << 1;
	p_value->preg = POINTER_CFG_REG;
	p_value->value = value;
	tmp102_read_word_reg(p_value);

	return 0;
}

uint8_t tmp102_read_temp_reg(uint8_t addr, uint16_t *value)
{
	p_value->addr = addr << 1;
	p_value->preg = POINTER_TMP_REG;
	p_value->value = value;
	tmp102_read_word_reg(p_value);

	return 0;
}

uint8_t tmp102_read_temp(uint8_t addr, uint16_t *value)
{
	tmp102_read_temp_reg(addr, value);
	*value >>= 3;	// data alignment: left-justified,
					// shift with 3-bit due to extended mode

	return 0;
}

uint8_t tmp102_read_temp_value(uint8_t addr, uint16_t *value)
{
	*value = 0;

	if(tmp102_write_config(p_config)) {
		uint16_t config = 0;

		do {
			tmp102_read_config_reg(ADDR_GND, &config);
			config &= 0x8000; // get OS bit
		} while ( config == 0x0  );
		tmp102_read_temp(ADDR_GND, value);
	}

	return 0;
}

uint8_t tmp102_read_temperature(uint16_t *value) {
	return tmp102_read_temp_value(ADDR_GND, value);
}


