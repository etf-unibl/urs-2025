/*
*******************************************************************************
Name 		: blinky.c
Author 		: Mladen Knezic
Description	: HPS_LED blinking controlled by HPS_KEY
*******************************************************************************
*/

#include "board_init.h"
#include "gpio.h"

// Bit position of the HPS LED and KEY pins in the GPIO1 module
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
	// Initial states
	unsigned int led_state = 0;   // 0 means LED is OFF, 1 means ON
	unsigned int is_blinking = 1; // Flag: 1 = blinking active, 0 = paused

	// Variables for tracking button state
	unsigned int current_key;
	unsigned int last_key = 1;    // Assuming initially not pressed (logical 1 due to pull-up)

	// Initialize the board
	board_init();

	// Set pin directions
	set_gpio_dir(GPIO_OUTPUT, HPS_LED); // LED is configured as output
	set_gpio_dir(GPIO_INPUT, HPS_KEY);  // Button is configured as input

	while (1)
	{
		/* * Break down the 250,000 spin cycles into 250 iterations of 1,000 cycles.
		 * The total delay is the same (~0.5s), but we poll the button
		 * 250 times during this period, making the system highly responsive.
		 */
		for (int i = 0; i < 250; i++)
		{
			// Read current button state
			current_key = read_gpio(HPS_KEY);

			// Detect falling edge (transition from 1 to 0 means button was just pressed)
			if (last_key == 1 && current_key == 0)
			{
				// Toggle the blinking state (1 becomes 0, 0 becomes 1)
				is_blinking ^= 1;

				if (is_blinking == 0)
				{
					led_state = 0;
					write_gpio(HPS_LED, led_state);
				}
			}

			// Update the previous button state for the next iteration
			last_key = current_key;

			// Short delay - acts as both waiting time and software button debounce
			spin(1000);
		}

		// After ~0.5s has passed (via the for loop), update the LED
		// only if blinking is currently active.
		if (is_blinking)
		{
			// Toggle the LED state
			led_state ^= 1;
			// Apply the new state to the physical pin
			write_gpio(HPS_LED, led_state);
		}
	}

	return 0;
}
