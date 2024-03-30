/**
*******************************************************************************
Name 		: gpio.h
Author 		: Mladen Knezic
Version 	: 0.1
Copyright 	: BSD License
Description	: Interface for GPIO functions
*******************************************************************************
*/

#ifndef GPIO_H_
#define GPIO_H_

/* ========================================================================== */
/*                               Macros & Typedefs                            */
/* ========================================================================== */
// Direction enum
typedef enum
{
	GPIO_INPUT,
	GPIO_OUTPUT
} gpio_dir_t;

/* ========================================================================== */
/*                                  Functions                                 */
/* ========================================================================== */
void set_gpio_dir(gpio_dir_t dir, unsigned int pin);
unsigned int read_gpio(unsigned int pin);
void write_gpio(unsigned int pin, unsigned int value);

#endif /* GPIO_H_ */
