#ifndef SENSOR_GPIO_H
#define SENSOR_GPIO_H

//#include "avr_compiler.h"
#include <stdint.h>
#include <stdbool.h>

void ss_hall_switch_init(void);
void ss_hall_switch_int_disable(void);
void ss_hall_switch_int_enable(void);
uint8_t ss_get_hall_switch_status(void);
void ss_set_hall_switch_status(uint8_t value);
uint8_t ss_get_hall_gpio_status(void);

#endif

/* EOF */

