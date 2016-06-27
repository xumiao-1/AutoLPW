/**
 * this is a file for learning and testing
 */

#include "mydef.h"
#include "stm32l1xx_rtc.h"
#include "hal_avr.h"
#include "tat.h"
#include "flash.h"
#include "include.h"
#include "startup.h"
#include "flash_funcs.h"

#include <time.h>

uint8_t dbg_msg[MAX_DBG_MSG];

node_config_t gConfig;
uint8_t gRSSI;
uint8_t imgbuf[BUFSIZE]; // for reprogram
uint16_t fsize;
uint16_t fchck;
uint8_t fpage;

void GPIO_config_all_unused(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	
    RCC_AHBPeriphClockCmd(
                          RCC_AHBPeriph_GPIOA | 
                          RCC_AHBPeriph_GPIOB | 
                          RCC_AHBPeriph_GPIOC |
                          RCC_AHBPeriph_GPIOD | 
                          RCC_AHBPeriph_GPIOE | 
                          RCC_AHBPeriph_GPIOH |
                          RCC_AHBPeriph_GPIOF | 
                          RCC_AHBPeriph_GPIOG
                           ,ENABLE);
//	/* Disable the Serial Wire Jtag Debug Port SWJ-DP */
//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE); 

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	/* PA  */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10
								| GPIO_Pin_11| GPIO_Pin_12| GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* PB  */
	#ifdef V13
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0| GPIO_Pin_3| GPIO_Pin_9;
	#else // V12
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_9;
	#endif
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* PC  */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;//GPIO_Pin_13; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* PD  */
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1; 
	//GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;//GPIO_Pin_0|GPIO_Pin_1; 
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_GPIOD, DISABLE);

	/* others */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    GPIO_Init(GPIOH, &GPIO_InitStructure);
    //GPIO_Init(GPIOF, &GPIO_InitStructure);
    //GPIO_Init(GPIOG, &GPIO_InitStructure);
    
    RCC_AHBPeriphClockCmd(
//                          RCC_AHBPeriph_GPIOA | 
//                          RCC_AHBPeriph_GPIOB | 
//                          RCC_AHBPeriph_GPIOC |
//                          RCC_AHBPeriph_GPIOD | 
                          RCC_AHBPeriph_GPIOE | 
                          RCC_AHBPeriph_GPIOH |
                          RCC_AHBPeriph_GPIOF | 
                          RCC_AHBPeriph_GPIOG
						  , DISABLE);
}

void config_rtc_wkup(uint32_t wkup) {
	uint32_t val = wkup-1; // RTC_WUTR value

	// enable wakeup interrupt
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	// configure the rtc to generate rtc wakeup event
	RTC_WakeUpCmd(DISABLE);
	while( (RTC->ISR&RTC_ISR_WUTWF)==0 );

	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits); // medium wakeup???	
	RTC_SetWakeUpCounter(val);
//	RTC_WakeUpCmd(ENABLE);
	RTC_ClearFlag(RTC_FLAG_WUTF);

	//node_update_next_alarm(); // update alarm value

	return;
}


void RTC_WKUP_IRQHandler(void) {
  if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
  {
	RTC_ClearITPendingBit(RTC_IT_WUT);// clear pending bit
	EXTI_ClearITPendingBit(EXTI_Line20); // MUST DO!

	// update next alarm
//	node_update_next_alarm();

	// interrupted by RTC
//	if (INTR_UNKNOWN == eint) eint = INTR_ALR;
  }
}

void RTC_Alarm_IRQHandler(void)
{
  if(RTC_GetITStatus(RTC_IT_ALRA) != RESET)
  {
	RTC_ClearITPendingBit(RTC_IT_ALRA);
	EXTI_ClearITPendingBit(EXTI_Line17);

	// update next alarm
//	node_update_next_alarm();

	// disable alarm
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	// enable auto wkup
	RTC_WakeUpCmd(ENABLE);
	RTC_ClearFlag(RTC_FLAG_WUTF);

	// interrupted by RTC
	if (INTR_UNKNOWN == eint) eint = INTR_ALR;
  }
}

void rtc_wkup_irq_init(void) {
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure EXTI Line20(RTC WKUP) to generate an interrupt on rising edge */
	EXTI_ClearITPendingBit(EXTI_Line20);
	EXTI_InitStructure.EXTI_Line = EXTI_Line20;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}

// usart interrupt init
void usart_irq_init(void) {
//  GPIO_InitTypeDef  GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;  
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_Init(GPIOA, &GPIO_InitStructure);

//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, GPIO_PinSource10);

  EXTI_ClearITPendingBit(EXTI_Line10);                           // clear pending bit
  EXTI_InitStructure.EXTI_Line = EXTI_Line10;                    // exti-line 10
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;            // interrupt mode
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;        // interrupt at falling edge
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
}

//void enable_hall_switch(void) {
//	GPIO_InitTypeDef GPIO_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	EXTI_InitTypeDef EXTI_InitStructure;
//
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA , ENABLE);//RCC_APB2Periph_AFIO
// 
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	
//	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
//
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
//
//	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure);
//}
//
//void disable_hall_switch(void) {
//	GPIO_InitTypeDef GPIO_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	EXTI_InitTypeDef EXTI_InitStructure;
//
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA , ENABLE);//RCC_APB2Periph_AFIO
// 
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	
//	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource0);
//
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
//
//	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure);
//}



void SYSCLKConfig_STOP(void)
{
	ErrorStatus HSEStartUpStatus;

	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);
	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if(HSEStartUpStatus == SUCCESS)
	{
    	/* Enable PLL */ 
    	RCC_PLLCmd(ENABLE);

    	/* Wait till PLL is ready */
	    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
	    {
	    }
	
	    /* Select PLL as system clock source */
	    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	
	    /* Wait till PLL is used as system clock source */
	    while(RCC_GetSYSCLKSource() != 0x0C)
	    {
	    }
	}
}

// event log operation
uint8_t gEvent; // event log type
void set_log(uint8_t e) {
	gEvent |= e;
}
void reset_log(void) {
	gEvent = 0;
}
uint8_t get_log(void) {
	return gEvent;
}

void put_log(sensor_rpt_data_t *data) {
	data->_event = gEvent;
	gEvent = 0;
}

// date/time ==> uint32_t
uint32_t dt2int(RTC_DateTypeDef date, RTC_TimeTypeDef time) {
	struct tm my_time;
	my_time.tm_year = date.RTC_Year+100;//RTC_Year+2000-1900;
	my_time.tm_mon = date.RTC_Month-1;//1-1;//
	my_time.tm_mday = date.RTC_Date;//1;//
	my_time.tm_hour = time.RTC_Hours;
	my_time.tm_min = time.RTC_Minutes;
	my_time.tm_sec = time.RTC_Seconds;

	return mktime(&my_time);
}

uint32_t rtc_getcounter(void) {
	RTC_DateTypeDef date;// = {0,RTC_Month_January,1,0};
	RTC_TimeTypeDef time;// = {0,0,0,RTC_H12_AM};

	RTC_GetTime(RTC_Format_BIN, &time);
	RTC_GetDate(RTC_Format_BIN, &date);
	return dt2int(date, time);
}

// uint32_t ==> date/time
void int2dt(uint32_t ticks, RTC_DateTypeDef *date, RTC_TimeTypeDef *time) {
	struct tm *ptm = localtime(&ticks); // get UTC time based on ticks

	date->RTC_WeekDay = ptm->tm_wday == 0 ? RTC_Weekday_Sunday : ptm->tm_wday;
	date->RTC_Month = ptm->tm_mon + 1;
	date->RTC_Date = ptm->tm_mday;
	date->RTC_Year = ptm->tm_year - 100; // struct tm base: 1900; RTC_DateTypeDef base: 2000

	time->RTC_Hours = ptm->tm_hour;
	time->RTC_Minutes = ptm->tm_min;
	time->RTC_Seconds = ptm->tm_sec;
	time->RTC_H12 = RTC_H12_AM;
}

//// adjust RTC date/time register
//void node_sync_clock(RTC_DateTypeDef* date, RTC_TimeTypeDef* time) {
//	RTC_SetTime(RTC_Format_BIN, time);
//	RTC_SetDate(RTC_Format_BIN, date);
//}

// update next alarm value
//void node_update_next_alarm(void) {
//	gConfig.nextAlarm = rtc_getcounter() + gConfig.APP_WKUP_INTV;
//	//RTC_TimeTypeDef wkupInt = {0, 0, 0, 0};
//	//wkupInt.RTC_Seconds = gConfig.APP_WKUP_INTV;
//	//RTC_GetTime(RTC_Format_BIN, &gConfig.nextAlarm); // current time
//	//time_add(&gConfig.nextAlarm, &wkupInt); // increased by an interval
//}

void config_rtc_alarm(void) {
	NVIC_InitTypeDef NVIC_InitStructure;
//	RTC_AlarmTypeDef RTC_AlarmStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
//	RTC_DateTypeDef alarm_date;
//	RTC_TimeTypeDef alarm_time;
					 
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

	// EXTI configuration 
	EXTI_ClearITPendingBit(EXTI_Line17);
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Enable AlarmA interrupt 
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	RTC_ClearFlag(RTC_FLAG_ALRAF);
}
//void node_set_alarm(uint32_t counter) {
////	NVIC_InitTypeDef NVIC_InitStructure;
//	RTC_AlarmTypeDef RTC_AlarmStructure;
////	EXTI_InitTypeDef EXTI_InitStructure;
//	RTC_DateTypeDef alarm_date;
//	RTC_TimeTypeDef alarm_time;
//					 
//	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
//
////	// EXTI configuration 
////	EXTI_ClearITPendingBit(EXTI_Line17);
////	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
////	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
////	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
////	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
////	EXTI_Init(&EXTI_InitStructure);
////
////	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
////	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
////	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
////	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
////	NVIC_Init(&NVIC_InitStructure);
//
//	// Set the alarmA Masks
//	int2dt(counter, &alarm_date, &alarm_time);
//	RTC_AlarmStructure.RTC_AlarmTime = alarm_time;
//	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay; // ignore the date
//	RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
//	RTC_AlarmStructure.RTC_AlarmDateWeekDay = 1;
//	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);
//
////	// Set AlarmA subseconds and enable SubSec Alarm : generate 8 interripts per Second 
////	//RTC_AlarmSubSecondConfig(RTC_Alarm_A, 0xFF, RTC_AlarmSubSecondMask_SS14_5);
////	// Enable AlarmA interrupt 
////	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
////
////	//Enable the alarmA 
////	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);	
//}
////void node_set_alarm(RTC_TimeTypeDef *pAlarm) {
//////	RTC_SetAlarm(tm);
//////	RTC_WaitForLastTask();
////
////	NVIC_InitTypeDef NVIC_InitStructure;
////	RTC_AlarmTypeDef RTC_AlarmStructure;
////	EXTI_InitTypeDef EXTI_InitStructure;
////					 
////	//RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
////	
////	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
////
////	// EXTI configuration 
////	EXTI_ClearITPendingBit(EXTI_Line17);
////	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
////	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
////	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
////	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
////	EXTI_Init(&EXTI_InitStructure);
////
////	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
////	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
////	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
////	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
////	NVIC_Init(&NVIC_InitStructure);
////
////	// Set the alarmA Masks
////	RTC_AlarmStructure.RTC_AlarmTime = *pAlarm;
////	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay; // ignore the date
////	RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
////	RTC_AlarmStructure.RTC_AlarmDateWeekDay = 1;
////	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);
////
////	// Set AlarmA subseconds and enable SubSec Alarm : generate 8 interripts per Second 
////	//RTC_AlarmSubSecondConfig(RTC_Alarm_A, 0xFF, RTC_AlarmSubSecondMask_SS14_5);
////	// Enable AlarmA interrupt 
////	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
////
////	//Enable the alarmA 
////	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);	
////}

//uint8_t time_add(RTC_TimeTypeDef *val, const RTC_TimeTypeDef *delta) {
//	uint8_t carry = 0;
//
//	// Seconds
//	val->RTC_Seconds += delta->RTC_Seconds;
//	if (val->RTC_Seconds >= 0x3C) {
//		val->RTC_Seconds -= 0x3C;
//		carry = 1;
//	} else {
//		carry = 0;
//	}
//
//	// Minutes
//	val->RTC_Minutes += delta->RTC_Minutes + carry;
//	if (val->RTC_Minutes >= 0x3C) {
//		val->RTC_Minutes -= 0x3C;
//		carry = 1;
//	} else {
//		carry = 0;
//	}
//
//	// Hours
//	val->RTC_Hours += delta->RTC_Hours + carry;
//	if (val->RTC_Hours >= 24) {
//		val->RTC_Hours -= 24;
//		carry = 1;
//	} else {
//		carry = 0;
//	}
//
//	return carry;
//}

//void data_print(uint8_t *msg, uint8_t len) {
//	uint8_t buf[64] = {0}; // buf[0]=0: data info
//	memcpy(buf+1, msg, len);
//	com_send_string(buf, len+1);
//}
//
//void debug_print(uint8_t *msg, uint8_t len) {
//	uint8_t buf[64] = {1}; // buf[0]=1: debug info
//	memcpy(buf+1, msg, len);
//	com_send_string(buf, len+1);
//}
void my_print(uint8_t type, uint8_t *msg, uint8_t len) {
	com_send_string(&type, sizeof(type));
	com_send_string(msg, len);
}

void save_config(node_config_t *config) {
	write_flash_with_addr_len(NODE_INFO_ADDR, (uint8_t*)config, sizeof(node_config_t));
//	delay_ms(1000);
}

void read_config(node_config_t *config) {
	read_flash(NODE_INFO_ADDR, sizeof(node_config_t), (uint8_t*)config);
}

// power on, off & reprogram
void node_power_on(void) {
	// change to STARTUP
	gConfig.gPhase = STARTUP;
	gConfig.nextAlarm = 0;//memset(&gConfig.nextAlarm, 0, sizeof(gConfig.nextAlarm));
	gConfig.APP_WKUP_INTV = 15;//30;
	srand(gConfig.gNodeAddr);
	gConfig.gRand = (uint8_t)(rand() & 0x00FF); // 0~255

	// modify bkp register
	// Enable the PWR clock, allow access to rtc
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);		
	PWR_RTCAccessCmd(ENABLE);
	// write
	RTC_WriteBackupRegister(RG_INDX, 0x0);
	RTC_WriteBackupRegister(RG_RESTART, FLAG_RESTART);

	// restart
	node_restart();
}

void node_power_off(void) {
	// change to NODEOFF
	gConfig.gPhase = NODEOFF;
	
	// modify bkp register
	// Enable the PWR clock, allow access to rtc
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);		
	PWR_RTCAccessCmd(ENABLE);
	// write
	RTC_WriteBackupRegister(RG_INDX, 0x0);
	RTC_WriteBackupRegister(RG_RESTART, FLAG_RESTART);

	// restart
	node_restart();
}

void node_reprogram(void) {
	// after reprogram, everything is reset
	gConfig.gPhase = STARTUP;
	
	// modify bkp register
	// Enable the PWR clock, allow access to rtc
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);		
	PWR_RTCAccessCmd(ENABLE);
	// write
	RTC_WriteBackupRegister(RG_INDX, FLAG_REPROGRAM);
	RTC_WriteBackupRegister(RG_RESTART, FLAG_RESTART);

	// restart
	node_restart();
}

static void node_restart(void) {
	// save node config
	save_config(&gConfig);
	
	// restart
	cli(); // disable interrupts
	NVIC_SystemReset();
}

uint16_t calc_checksum(uint8_t *saddr, uint16_t len) {
	uint16_t checksum = 0;
	for (; len > 0; len--, saddr++) {
		checksum += *saddr;
	}
	return checksum;
}

// for automation
/* config a pin */
#ifdef FULL_VER
void setup_pin_config(pin_config_t cfg) {
	GPIO_TypeDef *GPIOx;
	uint32_t RCC_AHBPeriph_GPIOx;
	GPIO_InitTypeDef GPIO_InitStructure;

	// which group?
	GPIOx = ((GPIO_TypeDef *)(AHBPERIPH_BASE + cfg.grp * 0x0400));
	RCC_AHBPeriph_GPIOx = RCC_AHBPeriph_GPIOA << cfg.grp;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOx, ENABLE);

	// which pin?
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 << cfg.pin;

	// which mode?
	if (cfg.config <= CFG_GP_OD_PD_1)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	else if (cfg.config <= CFG_IN_PD)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	else
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;

	// which speed
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;

	// which output type
	if (cfg.config <= CFG_GP_PP_PD_1) {
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	} else {
	  	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	}

	// pull up or pull down
	switch (cfg.config) {
	case CFG_GP_PP_0:
	case CFG_GP_PP_1:
	case CFG_GP_OD_0:
	case CFG_GP_OD_1:
	case CFG_IN_FLOAT:
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		break;

	case CFG_GP_PP_PU_0:
	case CFG_GP_PP_PU_1:
	case CFG_GP_OD_PU_0:
	case CFG_GP_OD_PU_1:
	case CFG_IN_PU:
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		break;

	case CFG_GP_PP_PD_0:
	case CFG_GP_PP_PD_1:
	case CFG_GP_OD_PD_0:
	case CFG_GP_OD_PD_1:
	case CFG_IN_PD:
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
		break;
	
	default:
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		break;
	}

	GPIO_Init(GPIOx, &GPIO_InitStructure);

	// output hi/low
	switch (cfg.config) {
	case CFG_GP_PP_0:
	case CFG_GP_PP_PU_0:
	case CFG_GP_PP_PD_0:
	case CFG_GP_OD_0:
	case CFG_GP_OD_PU_0:
	case CFG_GP_OD_PD_0:
		GPIO_ResetBits(GPIOx, GPIO_InitStructure.GPIO_Pin);
		break;

	case CFG_GP_PP_1:
	case CFG_GP_PP_PU_1:
	case CFG_GP_PP_PD_1:
	case CFG_GP_OD_1:
	case CFG_GP_OD_PU_1:
	case CFG_GP_OD_PD_1:
		GPIO_SetBits(GPIOx, GPIO_InitStructure.GPIO_Pin);
		break;

	default:
		break;
	}

}
#else
void setup_pin_config(pin_config_t cfg) {
	GPIO_TypeDef *GPIOx;
	uint32_t RCC_AHBPeriph_GPIOx;
	GPIO_InitTypeDef GPIO_InitStructure;

	// which group?
	GPIOx = ((GPIO_TypeDef *)(AHBPERIPH_BASE + cfg.grp * 0x0400));
	RCC_AHBPeriph_GPIOx = RCC_AHBPeriph_GPIOA << cfg.grp;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOx, ENABLE);

	// which pin?
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 << cfg.pin;

	// which mode?
	if (cfg.config <= CFG_GP_OD_PD_1)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	else
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;

	// which speed
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;

	// which output type
	if (cfg.config <= CFG_GP_PP_PD_1) {
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	} else {
	  	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	}

	// pull up or pull down
	switch (cfg.config) {
	case CFG_GP_PP_0:
	case CFG_GP_PP_1:
	case CFG_GP_OD_1:
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		break;

	case CFG_GP_PP_PU_0:
	case CFG_GP_OD_PU_1:
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		break;

	case CFG_GP_PP_PD_1:
	case CFG_GP_OD_PD_1:
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
		break;
	
	default:
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		break;
	}

	GPIO_Init(GPIOx, &GPIO_InitStructure);

	// output hi/low
	switch (cfg.config) {
	case CFG_GP_PP_0:
	case CFG_GP_PP_PU_0:
		GPIO_ResetBits(GPIOx, GPIO_InitStructure.GPIO_Pin);
		break;

	case CFG_GP_PP_1:
	case CFG_GP_PP_PD_1:
	case CFG_GP_OD_1:
	case CFG_GP_OD_PU_1:
	case CFG_GP_OD_PD_1:
		GPIO_SetBits(GPIOx, GPIO_InitStructure.GPIO_Pin);
		break;

	default:
		break;
	}

}
#endif

// config all pins
void setup_pin(uint8_t *pt) {
	// for loop: configure each pin
	int i, pin_num = pt[0];
	pin_config_t *cfg = (pin_config_t*)(pt+1);
	GPIO_InitTypeDef GPIO_InitStructure;

	// common config
    RCC_AHBPeriphClockCmd(
                          RCC_AHBPeriph_GPIOA | 
                          RCC_AHBPeriph_GPIOB | 
                          RCC_AHBPeriph_GPIOC |
                          RCC_AHBPeriph_GPIOD | 
                          RCC_AHBPeriph_GPIOE | 
                          RCC_AHBPeriph_GPIOH |
                          RCC_AHBPeriph_GPIOF | 
                          RCC_AHBPeriph_GPIOG
                           ,ENABLE);

	for (i=0; i<pin_num; i++) {
		setup_pin_config(*cfg);
		cfg++;
	}


	/* PC  */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* PD  */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	/* others */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    GPIO_Init(GPIOH, &GPIO_InitStructure);
    //GPIO_Init(GPIOF, &GPIO_InitStructure);
    //GPIO_Init(GPIOG, &GPIO_InitStructure);

    RCC_AHBPeriphClockCmd(
//                          RCC_AHBPeriph_GPIOA | 
//                          RCC_AHBPeriph_GPIOB | 
//                          RCC_AHBPeriph_GPIOC |
//                          RCC_AHBPeriph_GPIOD | 
                          RCC_AHBPeriph_GPIOE | 
                          RCC_AHBPeriph_GPIOH |
                          RCC_AHBPeriph_GPIOF | 
                          RCC_AHBPeriph_GPIOG
						  , DISABLE);
}

void unset_pin() {

	/* restore led */
	hal_en_net_led();
	hal_en_data_led();

    /* restore RF */
	GPIO_RF_config();
	hal_reset_flags( );
	trx_init();

	/* restore flash */
	hal_flash_power_config();
	flash_init();


//	// PA9/10
//	com_init(BUD_115200);

//	// PB10/11
//	iic_func_enable();

}

void GPIO_RF_config() {
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// Enable SCK, MOSI and MISO GPIO clocks 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);

	GPIO_PinAFConfig(HAL_PORT_SPI, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(HAL_PORT_SPI, GPIO_PinSource7, GPIO_AF_SPI1);
	GPIO_PinAFConfig(HAL_PORT_SPI, GPIO_PinSource6, GPIO_AF_SPI1);
	//GPIO_PinAFConfig(HAL_PORT_SPI, GPIO_PinSource4, GPIO_AF_SPI1);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;

	// SPI SCK pin configuration 
	GPIO_InitStructure.GPIO_Pin = HAL_SCK_PIN;
	GPIO_Init(HAL_PORT_SPI, &GPIO_InitStructure);
	// SPI  MOSI pin configuration 
	GPIO_InitStructure.GPIO_Pin =  HAL_MOSI_PIN;
	GPIO_Init(HAL_PORT_SPI, &GPIO_InitStructure);
	// SPI MISO pin configuration 
	GPIO_InitStructure.GPIO_Pin = HAL_MISO_PIN;
	GPIO_Init(HAL_PORT_SPI, &GPIO_InitStructure);
	// SPI NSS pin configuration 
	//GPIO_InitStructure.GPIO_Pin = HAL_SS_PIN;
	//GPIO_Init(HAL_PORT_SPI, &GPIO_InitStructure);

	// RST-PB5 SLP-PB6
	GPIO_InitStructure.GPIO_Pin = HAL_RF_RST | HAL_SLP_TR;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(HAL_PORT_SLP_TR, HAL_SLP_TR);
	GPIO_SetBits(HAL_PORT_RST, HAL_RF_RST);
	
	// SS-PA4
	GPIO_InitStructure.GPIO_Pin = HAL_SS_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA, HAL_SS_PIN);
	//GPIO_SetBits(GPIOB, GPIO_Pin_5);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//V1.1: PB4...;   V1.2: PB0
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	//GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// eof...
