#include "startup.h"
#include "net_layer.h"
#include "app_layer.h"
#include "net_p2p.h"
#include "flash.h"

//static volatile bool bHalf2;	// true if in 2nd half cycle
static volatile bool bSend;
static volatile bool bOperation;
static volatile bool bStop;

void TIM2_Config(uint16_t);
void check_nb_list(void);

uint32_t n_of_record = 0; // # of record in buf
uint8_t tmpbuf[BUFSIZE]; // one page size
uint32_t lastSync = 0; // last time for sync
uint32_t seqno = 0;











bool get_flag_send(void) {
	return bSend;
}
void set_flag_send(bool flag) {
	AVR_ENTER_CRITICAL_REGION();
	bSend = flag;
	AVR_LEAVE_CRITICAL_REGION();
}

bool get_flag_operation(void) {
	return bOperation;
}
void set_flag_operation(bool flag) {
	AVR_ENTER_CRITICAL_REGION();
	bOperation = flag;
	AVR_LEAVE_CRITICAL_REGION();
}

bool get_flag_stop(void) {
	return bStop;
}
void set_flag_stop(bool flag) {
	AVR_ENTER_CRITICAL_REGION();
	bStop = flag;
	AVR_LEAVE_CRITICAL_REGION();
}

// control the ON period per time in different phase
// unit: per 1 ms
// t>=1
void TIM2_Config(uint16_t t) {
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	// TIM2
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	
	NVIC_Init(&NVIC_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);

	TIM_TimeBaseStructure.TIM_Period = t-1;
	TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t)(SYS_CLK/TIM_FREQ_1KHZ)-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ClearFlag(TIM2, TIM_FLAG_Update); // clear interrupt flag
	TIM_ITConfig(TIM2, TIM_IT_Update,ENABLE); // timer interrupt enable
	TIM_Cmd(TIM2, ENABLE); // enable tim2
}


void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update)!= RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update); // clear flag

		if (STARTUP == gConfig.gPhase) {
			//node_stop();
			//node_operation();
			bOperation = true;
		} else if (RUNNING == gConfig.gPhase) {
			static uint8_t n = 0;

			if (N_WRKSTN == gConfig.gNodeType) { // base station
				// enable node operation
				if ((4 == n) && (INTR_ALR == eint) )
					bOperation = true; 

				// enable node stop
				if (9 == n) {
					eint = INTR_UNKNOWN;
					bStop = true;
				}
			} else { // sensor node
				// enable sending data
				bSend = true;

				// enable node operation
				if (4 == n)	bOperation = true; 

				// enable node stop
				if (9 == n) {
					eint = INTR_UNKNOWN;
					bStop = true;
				}
			}

			n = (++n) > 9 ? 0 : n; // increase
		} else if (REPROGM == gConfig.gPhase) {
			bOperation = true;
		}
	}
}

void node_stop(uint8_t *aInCfg) {
	// enter stop 
	enter_node_stop();

#ifdef AUTOMATION
    // set up pin based on msg
	setup_pin(aInCfg);
#else // ifndef AUTOMATION
	// unused pins
	GPIO_config_all_unused();
#endif // AUTOMATION

	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

	// now wake up
	leave_node_stop();
}


volatile intr_type_t eint = INTR_UNKNOWN;


// external interrupt from UART
void EXTI15_10_IRQHandler(void) {
	if(EXTI_GetITStatus(EXTI_Line10) != RESET) {

		/* Clear EXTI line10 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line10);

		if (INTR_UNKNOWN == eint) eint = INTR_KBD;
	}
}



// check nb list
void check_nb_list(void) {
	check_nb_tbl();
	//check_lqi();
}


/**
 * call it before enter to stop
 */
void testing(void) {
	//GPIO_InitTypeDef GPIO_InitStructure;

	// RF
	GPIO_ResetBits(HAL_PORT_SPI, HAL_SCK_PIN);
	GPIO_ResetBits(HAL_PORT_SPI, HAL_MISO_PIN);
	GPIO_ResetBits(HAL_PORT_SPI, HAL_MOSI_PIN);


	// Flash
	GPIO_ResetBits(GPIOB, PIN_SCK);
	GPIO_ResetBits(GPIOB, PIN_SO);
	GPIO_ResetBits(GPIOB, PIN_SI);

	// TMP102
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;//GPIO_Speed_40MHz;
//	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_11;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//GPIO_SetBits(GPIOB, GPIO_Pin_10);
	//GPIO_SetBits(GPIOB, GPIO_Pin_11);


	// UART

	
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_TIM4 | RCC_APB1Periph_SPI2, DISABLE);
//	RCC_AHBPeriphClockLPModeCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, DISABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_SYSCFG, DISABLE);
//
//
////	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
//
//	FLASH_SLEEPPowerDownCmd(ENABLE);
//	RCC_AHBPeriphClockLPModeCmd(RCC_AHBPeriph_FLITF|RCC_AHBPeriph_SRAM, DISABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
}

void untesting(void) {
    RCC_AHBPeriphClockCmd(
                          RCC_AHBPeriph_GPIOA | 
                          RCC_AHBPeriph_GPIOB | 
                          RCC_AHBPeriph_GPIOC |
                          RCC_AHBPeriph_GPIOD | 
                          RCC_AHBPeriph_GPIOE | 
                          RCC_AHBPeriph_GPIOH |
                          RCC_AHBPeriph_GPIOF | 
                          RCC_AHBPeriph_GPIOG
						  , ENABLE);
	//GPIO_InitTypeDef GPIO_InitStructure;

	// RF


	// Flash


	// TMP102


	// UART


	// other tests	
}

void enter_node_stop(void) {

	// turn off RF
	tat_enter_sleep_mode();
	hal_set_rst_high();
	HAL_SS_HIGH();

	// turn off flash
	//set_spi_flash_state(SPI_FLASH_OFF);
	GPIO_SetBits(GPIOB, PIN_RST);
	GPIO_SetBits(GPIOB, PIN_CS);

	flash_off();

	// testing...
	#ifdef MY_DEBUG
	hal_clear_net_led();
	hal_clear_data_led();
	#endif
	testing();
	RTC_WakeUpCmd(ENABLE);
}

/**
 * call it after wake up from stop mode
 */
void leave_node_stop(void) {
	// restore clock
	SYSCLKConfig_STOP();
	untesting();

	// restore components
	unset_pin();

	// RF
	tat_leave_sleep_mode();
	// flash
	flash_on();

	// set RF to RX state
	while((tat_set_trx_state(RX_AACK_ON)!=TAT_SUCCESS)) delay_ms(1);
	#ifdef MY_DEBUG
	hal_set_net_led();
	hal_set_data_led();
	#endif
}


//void reply_schd_req(pkt_app_t *pkt) {
//	sync_data_t data;
//	pkt_app_t rep = {0, APP_SCHD_REP, 0};

//	if (0==gConfig.nextAlarm) return;

//	rep.hdr.nodeid = gConfig.gNodeAddr;

//	RTC_GetDate(RTC_Format_BIN, &data.date);
//	RTC_GetTime(RTC_Format_BIN, &data.time);
//	data.nxtalrm = gConfig.nextAlarm;
//	data.intv = gConfig.APP_WKUP_INTV;
//	data.dura = APP_WKUP_DURA;
//	memcpy(rep.data, (uint8_t*)&data, sizeof(sync_data_t));
//	if (pkt) // reply to a specified node
//		node_send_app_command(pkt->hdr.nodeid, (uint8_t*)&rep, SIZEOF(sync_data_t), 0);
//	else // otherwise broadcast
//		node_send_app_command(BROADCAST_ADDRESS, (uint8_t*)&rep, SIZEOF(sync_data_t), 0);
//		
//}
//void send_schd_req(uint16_t dst) {
//	pkt_app_t req = {0, APP_SCHD_REQ, 0};
//	req.hdr.nodeid = gConfig.gNodeAddr;
//	node_send_app_command(dst, (uint8_t*)&req, sizeof(pkt_app_header_t), 0);
//}



// eof
