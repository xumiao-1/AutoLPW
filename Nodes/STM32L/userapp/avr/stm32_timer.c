#include "stm32_timer.h"
#include "stm32l1xx_rtc.h"

#include "com.h"
#include "mydef.h"


static uint8_t volatile hal_TimerUsart_flag;
static uint8_t hal_TimerUsart_running_flag;

uint8_t hal_rtc_count=0;
bool io_test_flag=false;

void hal_sysclk_init(void)
{
	//SystemInit();
	//hal_rtc_init(0,0);
}

void hal_rtc_init(void)
{
	RTC_InitTypeDef RTC_InitStructure;
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;

	/* Enable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);// Enable the PWR clock 

	/* Allow access to RTC */
	PWR_RTCAccessCmd(ENABLE);// Allow access to RTC

	/* Enable the LSE OSC */
	RCC_LSEConfig(RCC_LSE_ON);// Enable the LSE OSC

	/* Wait till LSE is ready */  
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{
	}

	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	/* Enable the RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC APB registers synchronisation */
	RTC_WaitForSynchro();

	// Configure the RTC data register and RTC prescaler 
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
	RTC_InitStructure.RTC_SynchPrediv  = 0xFF;
	RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;
	RTC_Init(&RTC_InitStructure);

	// Set the time to 00h 00mn 00s AM 
	RTC_TimeStruct.RTC_H12     = RTC_H12_AM;
	RTC_TimeStruct.RTC_Hours   = 0x00;
	RTC_TimeStruct.RTC_Minutes = 0x00;
	RTC_TimeStruct.RTC_Seconds = 0x00;  
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);

	// Miao
	// set date
	RTC_DateStruct.RTC_WeekDay = RTC_Weekday_Tuesday;
	RTC_DateStruct.RTC_Month = RTC_Month_January;
	RTC_DateStruct.RTC_Date = 1;
	RTC_DateStruct.RTC_Year	= 0; // year base: 2000
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);

	// config rtc alarm
	config_rtc_alarm();
}

void hal_TimerUsart_init(uint16_t TCN_vlaue)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	//NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = TCN_vlaue;
	TIM_TimeBaseStructure.TIM_Prescaler = 9999; // 7.3728M/1024->7200
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;   
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //选择向上计数
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);	// clear interrupt flag
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);	// timer interrupt enable
	// TIM_Cmd(TIM4, ENABLE); // timer enable
	TIM_Cmd(TIM4, DISABLE);

	hal_TimerUsart_running_flag = FLAG_TRUE;
	hal_TimerUsart_flag = FLAG_FALSE;
}

uint8_t hal_get_TimerUsart_flag(void)
{
	return (hal_TimerUsart_flag);
}

void hal_set_TimerUsart_flag(uint8_t tflag)
{
	hal_TimerUsart_flag = tflag;
}

uint8_t hal_get_TimerUsart_status(void)
{
	return hal_TimerUsart_running_flag;
}

// 停止TCC0
void hal_TimerUsart_disable(void)
{
	hal_TimerUsart_running_flag = FLAG_FALSE;
	TIM_Cmd(TIM4, DISABLE);
}

// 开启TCCO，参数决定CNT值置零与否
void hal_TimerUsart_enable(const uint8_t set_zero)
{
	hal_TimerUsart_running_flag = FLAG_TRUE;
	TIM_Cmd(TIM4, ENABLE);
}

void hal_UartTimer_OV_isr(void)
{
	hal_TimerUsart_flag=FLAG_TRUE;
	TIM_Cmd(TIM4, DISABLE);

	//#if (NODE_ADDRESS == NODE_WRKSTN)
	//if (N_WRKSTN == gConfig.gNodeType) {
//		check_uart_msg();
	//}
	//#endif
}

//#define FX_TIMA			TIM2
//#define FX_TIMA_RCC		RCC_APB1Periph_TIM2
//#define FX_TIMA_IRQ		TIM2_IRQn
//
//#define FX_TIMB			TIM3
//#define FX_TIMB_RCC		RCC_APB1Periph_TIM3
//#define FX_TIMB_IRQ		TIM3_IRQn

//static uint8_t fx_timA_running_flag, fx_timB_running_flag;
//static volatile uint8_t fx_timA_OV_flag, fx_timB_OV_flag;
//
//void ax_timA_init(uint16_t initValue)
//{
//	NVIC_InitTypeDef NVIC_InitStructure;
//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//	RCC_APB1PeriphClockCmd(FX_TIMA_RCC, ENABLE);
//	
//	NVIC_InitStructure.NVIC_IRQChannel = FX_TIMA_IRQ;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
//
//	TIM_TimeBaseStructure.TIM_Period = initValue;
//	TIM_TimeBaseStructure.TIM_Prescaler = 9999; // 7.3728M/1024->7200
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0;   
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//	TIM_TimeBaseInit(FX_TIMA, &TIM_TimeBaseStructure);
//	FX_TIMB->CNT = 0;
//	TIM_ClearFlag(FX_TIMA, TIM_FLAG_Update);
//	TIM_ITConfig(FX_TIMA, TIM_IT_Update, ENABLE);
//	TIM_Cmd(FX_TIMA, ENABLE);
//
//	fx_timA_running_flag = FLAG_FALSE;
//	fx_timA_OV_flag = FLAG_FALSE;
//}
//
//void ax_timA_enable(uint16_t initValue)
//{
//	FX_TIMA->ARR = (uint16_t)initValue;
//	FX_TIMA->CNT = 0;
//	fx_timA_running_flag = FLAG_TRUE;
//	fx_timA_OV_flag = FLAG_FALSE;
//	TIM_Cmd(FX_TIMA, ENABLE);
//}
//
//void ax_timA_continue(void)
//{
//	fx_timA_running_flag = FLAG_TRUE;
//	TIM_Cmd(FX_TIMA, ENABLE);
//}
//
//void ax_timA_disable(void)
//{
//	fx_timA_running_flag = FLAG_FALSE;
//	TIM_Cmd(FX_TIMA, DISABLE);
//}
//
//uint8_t ax_timA_get_ov_flag(void)
//{
//	return fx_timA_OV_flag;
//}
//
//void ax_timA_set_ov_flag(uint8_t tflag)
//{
//	fx_timA_OV_flag = tflag;
//}
//
//uint8_t ax_timA_get_status(void)
//{
//	return (fx_timA_running_flag);
//}
//
//void ax_timB_init(uint16_t initValue)
//{
//	NVIC_InitTypeDef NVIC_InitStructure;
//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//	RCC_APB1PeriphClockCmd(FX_TIMB_RCC, ENABLE);
//
//	NVIC_InitStructure.NVIC_IRQChannel = FX_TIMB_IRQ;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
//
//	TIM_TimeBaseStructure.TIM_Period = initValue;
//	TIM_TimeBaseStructure.TIM_Prescaler = 9999; // 7.3728M/1024->7200
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//	TIM_TimeBaseInit(FX_TIMB, &TIM_TimeBaseStructure);
//	FX_TIMB->CNT = 0;
//	TIM_ClearFlag(FX_TIMB, TIM_FLAG_Update);
//	TIM_ITConfig(FX_TIMB, TIM_IT_Update, ENABLE);
//	TIM_Cmd(FX_TIMB, ENABLE);
//
//	fx_timB_running_flag = FLAG_FALSE;
//	fx_timB_OV_flag = FLAG_FALSE;
//}
//
//void ax_timB_enable(uint16_t initValue)
//{
//	FX_TIMB->ARR = (uint16_t)initValue;
//	FX_TIMB->CNT = 0;
//	fx_timB_OV_flag = FLAG_FALSE;
//	TIM_Cmd(FX_TIMB, ENABLE);
//	fx_timB_running_flag = FLAG_TRUE;
//}
//
//void ax_timB_continue(void)
//{
//	fx_timB_running_flag = FLAG_TRUE;
//	TIM_Cmd(FX_TIMB, ENABLE);
//}
//
//void ax_timB_disable(void)
//{
//	fx_timB_running_flag = FLAG_FALSE;
//	TIM_Cmd(FX_TIMB, DISABLE);
//}
//
//uint8_t ax_timB_get_ov_flag(void)
//{
//	return fx_timB_OV_flag;
//}
//
//void ax_timB_set_ov_flag(uint8_t tflag)
//{
//	fx_timB_OV_flag = tflag;
//}
//
//uint8_t ax_timB_get_status(void)
//{
//	return (fx_timB_running_flag);
//}

void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update)!= RESET){
		TIM_ClearITPendingBit(TIM4, TIM_FLAG_Update);
		hal_UartTimer_OV_isr();
	}
}

//void TIM2_IRQHandler(void)
//{
//	if(TIM_GetITStatus(FX_TIMA, TIM_IT_Update)!= RESET){
//		TIM_ClearITPendingBit(FX_TIMA, TIM_FLAG_Update);
//		fx_timA_OV_flag = FLAG_TRUE;
//	}
//}
//
//void TIM3_IRQHandler(void)
//{
//	if(TIM_GetITStatus(FX_TIMB, TIM_IT_Update)!= RESET){
//		TIM_ClearITPendingBit(FX_TIMB, TIM_FLAG_Update);
//		fx_timB_OV_flag = FLAG_TRUE;
//	}
//}

//void hal_RTC_OV_isr(void)
//{
//	
//	hal_rtc_count++;
//	com_send_hex(hal_rtc_count);
//	if(hal_rtc_count<0)
//	{
//		hal_rtc_count=0;
//		io_test_flag=true;
//		//RCC_RTCCLKCmd(DISABLE);
//		RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
//	}
//	//RTC_WaitForLastTask();//Wait until last write operation on RTC registers has finished 
//		
//}

uint8_t get_rtc_count(void)
{
	return hal_rtc_count;
}

bool get_io_test_flag(void)
{
	return io_test_flag;
}

void clear_io_test_flag(void)
{
	io_test_flag=false;
}

