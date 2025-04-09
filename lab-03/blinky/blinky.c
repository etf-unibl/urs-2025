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
      //blinkaanje aktivno
	 int blink_enabled = 1;

	    // Stanja tastera (za detekciju promjene)
	 int prev_button_state = 1; // HIGH (nepritisnuto)
	 int curr_button_state;

	// Initialize the board (IOCSR, pinmux, and reset)
	board_init();

	// Set HPS LED as output pin
	set_gpio_dir(GPIO_OUTPUT, HPS_LED);

	// Set HPS KEY as input pin
    set_gpio_dir(GPIO_INPUT, HPS_KEY);

	while (1)
	{


		// Proveri trenutno stanje tastera
		curr_button_state = read_gpio(HPS_KEY);


		// Ako je taster pritisnut (LOW) i prethodno je bio HIGH
		 if (curr_button_state == 0 && prev_button_state == 1)
		  {
		            // Promeni stanje blinkanja
		            blink_enabled ^= 1;

		            // Mala pauza zbog debounce efekta
		            spin(1000);
		   }

		 prev_button_state = curr_button_state;


	 if (blink_enabled)
      {
			// Update HPS LED state
				write_gpio(HPS_LED, led_state);
				// Toggle the LED state
					led_state ^= 1;

	  }
		// Wait for approximately 0.5s
			spin(250000);


	}

	return 0;
}


