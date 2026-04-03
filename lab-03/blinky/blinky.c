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
#define HPS_KEY 	(25U)

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

	int prev_button_state = 0;
	int current_button_state = 0;

	// by default, blinking is on
	int is_blinking = 1;

	while (1)
	{
		if(is_blinking) {
			// Update HPS LED state
			write_gpio(HPS_LED, led_state);
			// Wait for approximately 0.5s
			spin(250000);
			// Toggle the LED state
			led_state ^= 1;
		}

		current_button_state = read_gpio(HPS_KEY);

		// Detect button press
		if(current_button_state == 0 && prev_button_state == 1) {
			is_blinking = !is_blinking;
		}

		prev_button_state = current_button_state;
	}

	return 0;
}


