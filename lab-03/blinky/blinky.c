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

#define HPS_LED		(24U)
#define HPS_KEY		(25U)

static inline void spin(volatile int count)
{
	while (count--)
	{
		asm("nop");
	}
}

int main(void)
{
	unsigned int led_state   = 0;
	unsigned int blinking    = 1;   // 1 = is blinking, 0 = turned off
	unsigned int key_prev    = 1;   // previous button state ,not pressed=HIGH
	unsigned int key_current    = 1;   // current button state

	board_init();

	// LED set to output
	set_gpio_dir(GPIO_OUTPUT, HPS_LED);
	// KEY set to input
	set_gpio_dir(GPIO_INPUT, HPS_KEY);

	while (1)
	{
		// Read current button state
		key_current = read_gpio(HPS_KEY);

		// Detect lower edge HIGH -> LOW = button has been pressed
		if (key_prev == 1 && key_current == 0)
		{
			blinking ^= 1;   // toggle blinking
		}

		// Save previous button state
		key_prev = key_current;

		if (blinking)
		{
			write_gpio(HPS_LED, led_state);
			spin(250000);
			led_state ^= 1;
		}
		else
		{
			// blinking is off so turn the LED off
			write_gpio(HPS_LED, 0);
			spin(10000);   // pause before checking state
		}
	}

	return 0;
}
