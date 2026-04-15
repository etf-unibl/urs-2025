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
	unsigned int blinking_active = 1;
	unsigned int last_kez_state = 1;

	// Initialize the board (IOCSR, pinmux, and reset)
	board_init();

	// Set HPS LED as output pin
	set_gpio_dir(GPIO_OUTPUT, HPS_LED);
	set_gpio_dir(GPIO_INPUT, HPS_KEY);


	while (1)
	{
		unsigned int currnet_kez_state = read_gpio(HPS_KEY);

		if(currnet_kez_state == 0 & last_kez_state == 1){
			blinking_active ^= 1;
			spin(5000);
		}
		last_kez_state = currnet_kez_state;

		// Update HPS LED state
		if(blinking_active){
			led_state ^= 1;
			write_gpio(HPS_LED, led_state);
			// Wait for approximately 0.5s
			spin(250000);
			// Toggle the LED state
		}else{
			write_gpio(HPS_LED, 0);
		}
	}

	return 0;
}


