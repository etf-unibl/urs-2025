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

// Bit positions in GPIO1 module (29..57 -> GPIO1[24] = HPS_LED, GPIO1[25] = HPS_KEY)
#define HPS_LED		(24U)
#define HPS_KEY		(25U)

// Kratka debounce pauza (možeš prilagoditi)
#define DEBOUNCE_DELAY	10000

static inline void spin(volatile int count)
{
	while (count--)
	{
		asm("nop");
	}
}

int main(void)
{
	unsigned int led_state = 0;          // trenutno stanje LED (0=ugašena)
	unsigned int blink_enabled = 1;      // 1 = trepće, 0 = miruje (LED ugašena)
	unsigned int prev_button;            // prethodno stanje tastera

	// Inicijalizacija ploče (IOCSR, pinmux, reset)
	board_init();

	// Podesi HPS_LED kao izlazni pin
	set_gpio_dir(GPIO_OUTPUT, HPS_LED);
	// Podesi HPS_KEY kao ulazni pin (podrazumevano je već ulaz, ali ne škodi)
	set_gpio_dir(GPIO_INPUT, HPS_KEY);

	// Pročitaj početno stanje tastera (pretpostavka: pull-up, nepritisnut = 1)
	prev_button = read_gpio(HPS_KEY);

	while (1)
	{
		// Trenutno stanje tastera
		unsigned int curr_button = read_gpio(HPS_KEY);

		// Detekcija pritiska (aktivno nisko: pritisak daje 0)
		if (curr_button == 0 && prev_button == 1)
		{
			// Promeni režim rada LED
			blink_enabled = !blink_enabled;

			// Jednostavna debounce pauza (može i bez nje)
			spin(DEBOUNCE_DELAY);
		}
		prev_button = curr_button;

		// Upravljanje LED diodom u zavisnosti od režima
		if (blink_enabled)
		{
			// Trepće – menja stanje svakih ~0.5 s
			write_gpio(HPS_LED, led_state);
			led_state ^= 1;               // invertuj za sledeći put
			spin(250000);                 // ~500 ms
		}
		else
		{
			// Miruje – LED ugašena (čisto da bude definisano)
			write_gpio(HPS_LED, 0);
			led_state = 0;               // resetuj stanje za sledeći ciklus treptanja
			spin(50000);                 // kraće čekanje da se taster osetljivo proverava
		}
	}

	return 0;
}
