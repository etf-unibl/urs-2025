/*
*******************************************************************************
Name 		: gpio.c
Author 		: Mladen Knezic
Version 	: 0.1
Copyright 	: BSD License
Description	: Implementation of GPIO functions
*******************************************************************************
*/

/* ========================================================================== */
/*                                   Includes                                 */
/* ========================================================================== */
#include "gpio.h"

/* ========================================================================== */
/*                               Global Variables                             */
/* ========================================================================== */
// Base address of GPIO1 module
volatile int* HPS_GPIO1_ptr = (int*) 0xFF709000;

/* ========================================================================== */
/*                                  Functions                                 */
/* ========================================================================== */
void set_gpio_dir(gpio_dir_t dir, unsigned int pin)
{
	if (GPIO_INPUT == dir)
	{
		// Clear the bit on pin position in direction register (digital input)
		*(HPS_GPIO1_ptr + 1) &= ~(1 << pin);
		// Enable debounce if input pin (register gpio_debounce)
		*(HPS_GPIO1_ptr + 18) &= ~(1 << pin);
	}
	else
	{
		// Set the bit on pin position in direction register (digital output)
		*(HPS_GPIO1_ptr + 1) |= (1 << pin);
	}
}

unsigned int read_gpio(unsigned int pin)
{
	// Set default to HIGH
	unsigned read_val = 1;

	// Read the specified GPIO pin state (register gpio_ext_porta)
	unsigned int value = *(HPS_GPIO1_ptr + 20);

	// If the bit on pin position is 0, then return LOW
	// Otherwise, we return HIGH, which is default
	if (0 == (value & (1 << pin)))
	{
		read_val = 0;
	}

	return read_val;
}

void write_gpio(unsigned int pin, unsigned int value)
{
	// Access port data register (gpio_swporta_dr)
	if (0 == value)
	{
		// Set the bit on pin position to LOW
		*HPS_GPIO1_ptr &= ~(1 << pin);
	}
	else
	{
		// Set the bit on pin position to HIGH
		*HPS_GPIO1_ptr |= (1 << pin);
	}
}
