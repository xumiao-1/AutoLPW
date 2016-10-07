/**
  ******************************************************************************
  * @file    Project/STM32L1xx_StdPeriph_Templates/main.c 
  * @author  MCD Application Team
  * @version V1.1.1
  * @date    13-April-2012
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mydef.h"
#include "stm32_timer.h"
#include "sensor_tmp102.h"
#include "net_p2p.h"
#include "startup.h"
#include "sensor_gpio.h"
#include "app_layer.h"


/** @addtogroup Template_Project
  * @{
  */

/* Private macro -------------------------------------------------------------*/
#define NOT_RESET_PWR_MONITOR

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
	// system init	
	system_init();
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);

	#ifdef MY_DEBUG
	hal_set_net_led();
	hal_set_data_led();
	#endif
	
#if (NODE_ADDR == SNR_NODE)	// sensor node
	// config rtc
	rtc_wkup_irq_init();

#ifdef AUTOMATION
{
	s_sensor_pkt_t msg;
	pkt_app_t pkt = {NODE_ADDR, AUTO_CMD, 0xFF};
	uint8_t lCfg[CONFIG_LEN]; // pin configuration
#if defined(NOT_RESET_PWR_MONITOR) // not reset power monitor
	// wait command from PC
	while (1) {
		if (get_app_pool_items() != 0) {
			pkt_app_t *curPkt = get_app_pool_handle(); // get next pkt
			if (AUTO_CMD == curPkt->hdr.cmd 
					&& PC_CMD_ACT == curPkt->data[0]) {
				sync_data1_t *dt = (sync_data1_t*)(curPkt->data+1);
				config_rtc_wkup(dt->intv);
				//memcpy(lCfg, curPkt->data+1, curPkt->data[1]*2+1);
				break;
			}
		}
	}
#endif // NOT_RESET_PWR_MONITOR

	// step 1: tell pc that I'm ready
	// pkt data
	msg.cmd = SNR_CMD_RDY;
	msg.res = 0xFF;
	memcpy(pkt.data, (uint8_t*)&msg, sizeof(s_sensor_pkt_t));	
	delay_ms(100);
	node_send_app_command(PRX_NODE, (uint8_t*)&pkt, SIZEOF(msg), 2);
	hal_clear_net_led();

	// step 2: wait for reply from pc
	while (1) {
		if (get_app_pool_items() != 0) {
			pkt_app_t *curPkt = get_app_pool_handle(); // get next pkt
			if (AUTO_CMD == curPkt->hdr.cmd 
					&& PC_CMD_CFG == curPkt->data[0]) {
				memcpy(lCfg, curPkt->data+1, curPkt->data[1]*2+1);
				hal_set_net_led();
				break;
			}
		}
	}

	// step 3: get pin configuration, so setup pin

	// step 4: tell pc i'm ready to go to low power mode
	// data
	msg.cmd = SNR_CMD_LPW_BGN;
	msg.res = 0xFF;
	memcpy(pkt.data, (uint8_t*)&msg, sizeof(msg));
	delay_ms(100);
	node_send_app_command(PRX_NODE, (uint8_t*)&pkt, SIZEOF(msg), 2);

	// enter stop
	node_stop(lCfg);

	// step 5: wake up, report to pc
	msg.cmd = SNR_CMD_LPW_END;
	msg.res = 0xFF;
	memcpy(pkt.data, (uint8_t*)&msg, sizeof(msg));
	node_send_app_command(PRX_NODE, (uint8_t*)&pkt, SIZEOF(msg), 2);
}
#else // ifndef AUTOMATION
    // wkup every 60s
	config_rtc_wkup(60);

	// go to low power
	node_stop(NULL);

	node_send_app_command(PRX_NODE, "hello", 5, 2);
#endif //AUTOMATION

#if defined(NOT_RESET_PWR_MONITOR) // not reset power monitor
	// restart
	cli(); // disable interrupts
	NVIC_SystemReset();
#endif // NOT_RESET_PWR_MONITOR

#else // proxy node
	while(true)	{
		uint8_t len_recv_data;

		// get rf msg from sensor node
		if (get_app_pool_items() != 0) {
			pkt_app_t *curPkt = get_app_pool_handle(); // get next pkt

			my_print(DATA_MSG, curPkt->data, sizeof(s_sensor_pkt_t));

			if (AUTO_CMD == curPkt->hdr.cmd) {
				switch (curPkt->data[0]) {
				case SNR_CMD_RDY:
					hal_set_net_led();
					hal_clear_data_led();
					break;

				case SNR_CMD_LPW_BGN:
					hal_clear_net_led();
					hal_set_data_led();
					break;

				case SNR_CMD_LPW_END:
					hal_set_net_led();
					hal_set_data_led();
					break;

				default:
					break;
				}
			}
		}

		// get com msg from pc
		len_recv_data = com_get_number_of_received_bytes();
		if ( (len_recv_data>=1) && (len_recv_data<=COM_RX_BUFFER_SIZE) ) {
		    uint8_t *pt = com_get_received_data(); // cmd from pc
			pkt_app_t pkt = {NODE_ADDR, AUTO_CMD, 0xFF};
			memcpy(pkt.data, pt, len_recv_data);

			my_print(DBUG_MSG, pkt.data, len_recv_data); // for debug purpose

			// send to sensor node thru. rf
			node_send_app_command(SNR_NODE, 
								(uint8_t*)&pkt, 
								sizeof(pkt_app_header_t)+len_recv_data, 
								2);

			com_reset_receiver();
			len_recv_data = 0;
		} 
	}
#endif // (NODE_ADDR == SNR_NODE)
}

//**************************************************************//
  





/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
