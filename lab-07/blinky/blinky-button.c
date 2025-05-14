/*
*******************************************************************************
Name 		: blinky.c
Author 		: Mladen Knezic
Version 	: 0.1
Copyright 	: BSD License
Description	: Linux HPS_LED blinking example C program
*******************************************************************************
*/

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define GPIO1_REGS_BASE         (0xff709000)
#define GPIO1_REGS_SIZE         (0x00001000)
#define GPIO1_REGS_DATA_OFFSET  (0x0)
#define GPIO1_REGS_DIR_OFFSET   (0x4)
#define GPIO1_DEBOUNCE_OFFSET 	(0x48)
#define GPIO1_REGS_IN_OFFSET	(0x50)

#define HPS_LED_PIN	        (24U)
#define HPS_KEY_PIN		(25U)

unsigned int blinkingEnabled = 0;
pthread_mutex_t threadMutex = PTHREAD_MUTEX_INITIALIZER;

void* read_button(void* arg);

volatile unsigned int* hps_led_dir_addr = NULL;
volatile unsigned int* hps_led_port_addr = NULL;
volatile unsigned int* hps_key_port_addr = NULL;
volatile unsigned int* debounce_addr = NULL;

int main(void)
{
    void* virtual_base;
    int fd;

    // Open /dev/mem
    fd = open("/dev/mem", (O_RDWR | O_SYNC));

    if (-1 == fd)
    {
        printf("Error when opening /dev/mem! Exiting...\n");
        return 1;
    }

    // Get virtual address that maps to physical address for GPIO1
    virtual_base = mmap(NULL, GPIO1_REGS_SIZE, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, GPIO1_REGS_BASE);

    if (MAP_FAILED == virtual_base)
    {
        printf("Error when trying to mmap virtual to physical GPIO address! Exiting...\n");
        close(fd);
        return 1;
    }

    // Get the address that maps to the HPS_LED direction register
    hps_led_dir_addr = (unsigned int *)(virtual_base + GPIO1_REGS_DIR_OFFSET);
    // Get the address that maps to the HPS_LED port register
    hps_led_port_addr = (unsigned int *)(virtual_base + GPIO1_REGS_DATA_OFFSET);
    hps_key_port_addr = (unsigned int *)(virtual_base + GPIO1_REGS_IN_OFFSET);

    debounce_addr = (unsigned int *)(virtual_base + GPIO1_DEBOUNCE_OFFSET);
    
    // Set HPS_LED direction to be output
    *hps_led_dir_addr |= 1 << HPS_LED_PIN;
    
    // Turn the HPS_LED off initially
    *hps_led_port_addr |= 1 << HPS_LED_PIN;

    // Set HPS_KEY direction to be input
    *hps_led_dir_addr &= ~(1 << HPS_KEY_PIN);
    *debounce_addr &= ~(1 << HPS_KEY_PIN);


    pthread_t buttonThread;
    if(pthread_create(&buttonThread, NULL, read_button, NULL) != 0)
    {
	    perror("Thread creation failed");
	    return 1;
    }
    // Loop infinitely and toggle the HPS_LED every 0.5 seconds
    while(1)
    {
	    pthread_mutex_lock(&threadMutex);
	    if(blinkingEnabled)
	    {
		    int state = *hps_led_port_addr >> HPS_LED_PIN & 1;
		    if(state == 0)
		    {
			    *hps_led_port_addr |= 1 << HPS_LED_PIN;
		    }
		    else
		    {
			    *hps_led_port_addr &= ~(1 << HPS_LED_PIN);
		    }
	    }
	    pthread_mutex_unlock(&threadMutex);
	    usleep(500000);
    }

    // Unmap previously mapped virtual address space
    if(0 != munmap(virtual_base, GPIO1_REGS_SIZE))
    {
        printf("Error when trying to munmap previously mapped addresses! Exiting...\n");
        close(fd);
        return 1;
    }

    // Close the file descriptor
    close(fd);

    return 0;
}


void* read_button(void* arg)
{
	int prevButtonState = 1;
	while(1)
	{
		pthread_mutex_lock(&threadMutex);
		unsigned int buttonState = *hps_key_port_addr & (1 << HPS_KEY_PIN);

		if(prevButtonState != 0 && buttonState == 0)
		{
			blinkingEnabled = ~blinkingEnabled;
			prevButtonState = 0;
		}
		else if (buttonState && prevButtonState == 0)
		{
			prevButtonState = 1;
		}
		pthread_mutex_unlock(&threadMutex);
		usleep(2000);
	}
}
