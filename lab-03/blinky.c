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

	// Initialize the board (IOCSR, pinmux, and reset)
	board_init();

	// Set HPS LED as output pin
	set_gpio_dir(GPIO_OUTPUT, HPS_LED);
	set_gpio_dir(GPIO_INPUT, HPS_KEY);

	unsigned int button_state, prev_but_state = 1, toggle_on = 0;

	while (1)
	{
		// Update HPS LED state
		write_gpio(HPS_LED, led_state);
		button_state = read_gpio(HPS_KEY);
		// Wait for approximately 0.5s

		if(prev_but_state == 1 && button_state == 0)
		{
			toggle_on ^= 1;
		}
		prev_but_state = button_state;
		if(toggle_on) {
			spin(250000);
			led_state ^= 1;
		}
		//led_state = button_state;
	}

	return 0;
}


