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

	unsigned int key_prev = 1;
	unsigned int flag     = 0;

	// Initialize the board (IOCSR, pinmux, and reset)
	board_init();

	// Set HPS LED as output pin
	set_gpio_dir(GPIO_OUTPUT, HPS_LED);

	while (1)
	{
		unsigned int key_curr = read_gpio(HPS_KEY);

		if(key_curr == 0 && key_prev == 1)
		{
			key_prev = 0;
			flag     = !flag;

			if (flag == 1)
			{
				// Update HPS LED state
				write_gpio(HPS_LED, led_state);
				// Wait for approximately 0.5s
				spin(250000);
				// Toggle the LED state
				led_state ^= 1;
			}
			else
			{
				write_gpio(HPS_LED, 0);
			}
		}

		if(key_curr == 1 && key_prev == 0)
		{
			key_prev = 1;
		}

	}

	return 0;
}


