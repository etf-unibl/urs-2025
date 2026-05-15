/*
*******************************************************************************
Name 		: blinky.c
Author 		: Mladen Knezic
Version 	: 0.1
Copyright 	: BSD License
Description	: Linux HPS_LED blinking example C program
*******************************************************************************
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

#define GPIO1_REGS_BASE         (0xff709000)
#define GPIO1_REGS_SIZE         (0x00001000)
#define GPIO1_REGS_DATA_OFFSET  (0x0)
#define GPIO1_REGS_DIR_OFFSET   (0x4)
#define GPIO1_REGS_EXT_PORT_OFFSET (0x50)

#define HPS_LED_PIN	        (24U)
#define HPS_BUTTON_PIN          (25U)

volatile unsigned int* gpio_dir_addr = NULL;
volatile unsigned int* gpio_port_addr = NULL;
volatile unsigned int* gpio_ext_port_addr = NULL;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int blink_enabled = 0;
static int running = 1;

void* button(void* arg)
{
	unsigned int previous_button_state = 1;
	
	while(running) 
	{
		unsigned int button_state;
		
		button_state = (*gpio_ext_port_addr >> HPS_BUTTON_PIN) & 1U;
	
		if(previous_button_state == 1 && button_state == 0)
		{
			pthread_mutex_lock(&mutex);
				blink_enabled = !blink_enabled;
			pthread_mutex_unlock(&mutex);
			
			usleep(200000);
		}
		
		previous_button_state = button_state;
		usleep(20000);
	}
	
	return NULL;
}

void* led(void* arg)
{
	while(running)
	{
		int led_blink_enabled;
		pthread_mutex_lock(&mutex);
			led_blink_enabled = blink_enabled;
		pthread_mutex_unlock(&mutex);
		
		if(led_blink_enabled)
		{
			*gpio_port_addr |= (1U << HPS_LED_PIN); // LED on
			usleep(500000);
	
			*gpio_port_addr &= ~(1U << HPS_LED_PIN); // LED off
			usleep(500000);
		}
		else
		{
			*gpio_port_addr &= ~(1U << HPS_LED_PIN);
			usleep(100000);
		}
	}
	
	return NULL;
}

int main(void)
{
    void* virtual_base;
    int fd;
    
    pthread_t button_thread;
    pthread_t led_thread;

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
    gpio_dir_addr = (volatile unsigned int *)(virtual_base + GPIO1_REGS_DIR_OFFSET);
    // Get the address that maps to the HPS_LED port register
    gpio_port_addr = (volatile unsigned int *)(virtual_base + GPIO1_REGS_DATA_OFFSET);
    
    gpio_ext_port_addr = (volatile unsigned int*)(virtual_base + GPIO1_REGS_EXT_PORT_OFFSET);
    
    // Set HPS_LED direction to be output
    *gpio_dir_addr |= (1U << HPS_LED_PIN);
    // Set HPS_BUTTON direction to be input
    *gpio_dir_addr &= ~(1U << HPS_BUTTON_PIN);
    
    // Turn the HPS_LED off initially
    *gpio_port_addr &= ~(1U << HPS_LED_PIN);

     if (pthread_create(&button_thread, NULL, button, NULL) != 0)
    {
    	printf("Error when creating button thread!\n");
    	munmap(virtual_base, GPIO1_REGS_SIZE);
    	close(fd);
    	return 1;
    }
    
    if (pthread_create(&led_thread, NULL, led, NULL) != 0)
    {
    	printf("Error when creating led thread!\n");
    	running = 0;
    	pthread_join(button_thread, NULL);
    	munmap(virtual_base, GPIO1_REGS_SIZE);
    	close(fd);
    	return 1;
    }

    
    pthread_join(button_thread, NULL);
    pthread_join(led_thread, NULL);
    
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

