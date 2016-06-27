/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief This files implements the API for the communications library for the 
 *        FTDI chip on the STK541 board and the AVR USART used for the RZ502 
 *        version.
 *
 *      This file contains the function prototypes for the Transceiver Access 
 *      Toolbox, hence it is an API. The Transceiver Access Toolbox is an
 *      abstraction layer that hides the details of the radio transceiver from 
 *      the end-user. The goal for the Transceiver Access Toolbox is to wrap the 
 *      services that the radio transceiver can perform into easy to use functions.
 *
 * \par Application note:
 *      AVR2001: Transceiver Access Toolbox for the AT86RF230
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler 
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 * 
 * $Name$
 * $Revision: 613 $
 * $RCSfile$
 * $Date: 2006-04-07 14:40:07 +0200 (fr, 07 apr 2006) $  \n
 *
 * Copyright (c) 2006, Atmel Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of ATMEL may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
#ifndef COM_H
#define COM_H
/*============================ INCLDUE =======================================*/
#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"
/*============================ MACROS ========================================*/
//#define ENABLE_RECEIVER  ( UCSR0B |= ( 1 << RXEN0 ) ) /*!< Enables receiver. */
//#define DISABLE_RECEIVER ( UCSR0B &= ~( 1 << RXEN0 ) ) /*!< Disables receiver. */

//#define ENABLE_TRANSMITTER  ( UCSR0B |= ( 1 << TXEN0 ) ) /*!< Enables transmitter. */
//#define DISABLE_TRANSMITTER ( UCSR0B &= ~( 1 << TXEN0 ) ) /*!< Disables transmitter. */

//#define ENABLE_RECEIVE_COMPLETE_INTERRUPT  ( UCSR0B |= ( 1 << RXCIE0 ) ) /*!< Enables an interrupt each time the receiver completes a symbol. */
//#define DISABLE_RECEIVE_COMPLETE_INTERRUPT ( UCSR0B &= ~( 1 << RXCIE0 ) ) /*!< Disables an interrupt each time the receiver completes a symbol. */

/*#define LOW ( 0x00 )
#define XRAM_ENABLE( )     XMCRA |= ( 1 << SRE ); XMCRB |= ( 1 << XMBK )
#define XRAM_DISABLE( )    XMCRA &= ~( 1 << SRE )*/


/*============================ TYPEDEFS ======================================*/
/*============================ VARIABLES =====================================*/
/*============================ PROTOTYPES ====================================*/


#define BUD_4800         2400
#define BUD_9600         4800
#define BUD_19200       9600
#define BUD_38400       19200
#define BUD_57600       28800
#define BUD_115200     57600




//void com_init( baud_rate_t rate, bool open_flag ,uint8_t Timer0value);
void com_init( uint32_t band_rate);
void com_send_string( uint8_t *data, uint8_t data_length );
void com_send_hex( uint8_t nmbr );
void com_send_char( uint8_t data );
uint8_t * com_get_received_data( void );
uint8_t com_get_number_of_received_bytes( void );
void com_reset_receiver( void );

 bool Get_com_Rx_Finished_flag(void);
 void  Clear_com_Rx_Finished_flag(void );
 void Set_com_Rx_Finished_flag( void );
 bool get_ht_flag(void);

  void uart_rxne_isr(void);
  
#endif
