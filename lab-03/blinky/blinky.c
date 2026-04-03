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
#define HPS_KEY     (25U)

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
	unsigned int button_state = 1;
	int mode = 1;

	// Initialize the board (IOCSR, pinmux, and reset)
	board_init();

	// Set HPS LED as output pin
	set_gpio_dir(GPIO_OUTPUT, HPS_LED);
	set_gpio_dir(GPIO_INPUT, HPS_KEY);

	while (1)
	{
		unsigned int button_now = read_gpio(HPS_KEY);

		// button press
		if ((button_now == 0) && (button_state == 1))
		{
			mode = !mode ;
			button_state = 0;
		}

		// button release
		if ((button_now == 1) && (button_state == 0))
		{
			button_state = 1;
		}

		if (mode == 1)
		{
			// Update HPS LED state
			write_gpio(HPS_LED, led_state);
			// Wait for approximately 0.5s
			spin(250000);
			// Toggle the LED state
			led_state ^= 1;
		}
		else
			write_gpio(HPS_LED, 0);

	}

	return 0;
}


