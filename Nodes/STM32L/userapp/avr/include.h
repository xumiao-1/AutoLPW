#ifndef INCLUDE_H
#define INCLUDE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <macros.h>
//#include <util/delay.h>
//#include<avr/sleep.h>

#include "config_uart_extended.h" // See this file for all project options.
#include "compiler.h"
#include "rf_type.h"

#if(RF_PLATFORM==ZGB_LINK_230)
#include "at86rf230.h"
#elif(RF_PLATFORM==ZGB_LINK_212)
#include "at86rf212.h"
#elif(RF_PLATFORM==ZGB_LINK_231)
#include "at86rf231.h"
#endif

#include "hal.h"
#include "hal_avr.h"
#include "com.h"
#include "function.h"
//#include "user_config.h"
#include "tat.h"
#include "compiler_avr.h"
#include "user_function.h"

// Miao
#define MY_DEBUG
#define V13

#endif
