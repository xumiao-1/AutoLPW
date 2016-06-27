#include "user_function.h"

#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>


static  uint32_t TimingDelay = 0;

void reset(void)
{
	(*((void(*)(void))PROG_START))();
}


uint16_t crc_ccitt_update(uint16_t crc, uint8_t data)
{
	data ^= (uint8_t)(crc);
	data ^= data << 4;

	return ((((uint16_t)data << 8) | (uint8_t)(crc>>8)) ^ (uint8_t)(data >> 4) 
													^ ((uint16_t)data << 3));
}

uint8_t XorGeneration(uint8_t *data, const uint8_t len)
{
	uint8_t i, xor_tmp=0;
	for(i=0;i<len;i++, data++){
		xor_tmp ^= (*data);
	}
	return xor_tmp;
}

void bufcpy(uint8_t *dest, uint8_t *src, uint16_t len)
{
	while(len != 0){
		*dest=*src;
		dest ++, src ++, len --;
	}
}

void delay_us(uint16_t us)
{
	uint8_t i;
	while(us-- > 0){
		i = 10;
		while(i--);
	}
}

void delay_ms(uint16_t ms)
{
	while(ms-- > 0){
		delay_us(250);
	}
}
/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void DecrementTimingDelay(void)
{
  if (TimingDelay != 0x00)
  {
    TimingDelay--;
  }
}

/*******************************************************************************
* Function Name  : SysTick_Configuration
* Description    : Configure a SysTick Base time to 10 ms.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTick_Configuration(void)
{
  // Setup SysTick Timer for 10 msec interrupts  
/*  if (SysTick_Config(SystemCoreClock / 100))
  { 
    // Capture error 
    while (1);
  }
  
 // Configure the SysTick handler priority 
  NVIC_SetPriority(SysTick_IRQn, 0x0); */
}	



