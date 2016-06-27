
#ifndef COMPILER_AVR_H
#define COMPILER_AVR_H

#include "stm32l1xx.h"
#include <stdbool.h>
//#define false 	FALSE
//#define true 	TRUE


//#define sei()	__set_PRIMASK(0)
//#define cli()	__set_PRIMASK(1)

#define sei()	__enable_irq()
#define cli()	__disable_irq()


#define INLINE static inline

#define AVR_ENTER_CRITICAL_REGION()	cli()
#define AVR_LEAVE_CRITICAL_REGION() sei()

#endif
