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
	// HPS LED initial state is ON
	unsigned int led_state = 1;

	// Initialize the board (IOCSR, pinmux, and reset)
	board_init();

	// Set HPS LED and HPS KEY as output pin
	set_gpio_dir(GPIO_OUTPUT, HPS_LED);
	set_gpio_dir(GPIO_INPUT, HPS_KEY);

	char button_pressed = 0x00;

	while (1)
	{
		write_gpio(HPS_LED, led_state);

		spin(150000); 	// 0.5s

		led_state ^= 1;	// Toggle the LED state

		unsigned int button_state = read_gpio(HPS_KEY);

		if(button_state == 0) // button pressed
		{
			button_pressed = 0xff;
 			write_gpio(HPS_LED, 0x00);


 			while(1)
 			{
 				if(read_gpio(HPS_KEY) == 1)
 					button_pressed = 0x00;
 				else if (read_gpio(HPS_KEY) == 0 && button_pressed == 0x00 )
 					break;
 				else
 					asm("nop");
 			}
 		}


	}

	return 0;
}


