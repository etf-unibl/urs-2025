/*
*******************************************************************************
Name 		: blinky.c
Author 		: Mladen Knezic
Version 	: 0.1
Copyright 	: BSD License
Description	: HPS_LED blinking example C program
*******************************************************************************
*/

#include "board_init.h"
#include "gpio.h"

// Bit position of the HPS LED pin in the GPIO1 module
#define HPS_LED		(24U)
// Bit position of the HPS KEY pin in the GPIO1 module
#define HPS_KEY		(25U)

// Busy-wait function
static inline void spin(volatile int count)
{
	while (count--)
	{
		asm("nop");
	}
}

int main(void)
{
	// HPS LED initial state is OFF
	unsigned int led_state = 0;
	// Blinking flag (1 = blinking, 0 = stopped)
	unsigned int is_blinking = 1;

	// Key states (1 means released/unpressed, 0 means pressed)
	unsigned int current_key = 1;
	unsigned int last_key = 1;

	// Initialize the board (IOCSR, pinmux, and reset)
	board_init();

	// Set HPS LED as output pin
	set_gpio_dir(GPIO_OUTPUT, HPS_LED);
	// Set HPS KEY as input pin
	set_gpio_dir(GPIO_INPUT, HPS_KEY);

	while (1)
	{
		current_key = read_gpio(HPS_KEY);

		if (last_key == 1 && current_key == 0)
		{
			if (is_blinking == 0)
			{
				is_blinking = 1;
			}
			else
			{
				is_blinking = 0;
			}
		}

		last_key = current_key;

		if (is_blinking == 1)
		{
			write_gpio(HPS_LED, led_state);

			if (led_state == 0)
			{
				led_state = 1;
			}
			else
			{
				led_state = 0;
			}
		}

		spin(250000);
	}

	return 0;
}
