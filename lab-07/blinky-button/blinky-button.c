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

#define GPIO1_REGS_BASE               (0xff709000)
#define GPIO1_REGS_SIZE               (0x00001000)
#define GPIO1_REGS_DATA_OFFSET        (0x0)
#define GPIO1_REGS_DIR_OFFSET         (0x4)

#define GPIO1_REGS_EXT_PORTA_OFFSET   (0x50)

#define HPS_LED_PIN	        (24U)
#define HPS_KEY_PIN         (25U)
#define HPS_KEY_MASK        (1U << HPS_KEY_PIN)

// User manual page 87 

volatile unsigned int* gpio_swporta_dr  = NULL; // Used to write output data to output I/O pin
volatile unsigned int* gpio_swporta_ddr = NULL; // Used to configure the direction of I/O pin
volatile unsigned int* gpio_ext_porta   = NULL; // Used to read input data of I/O pin

int led_on = (0U);
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void set_state()
{
	*gpio_swporta_dr |= (1U << HPS_LED_PIN);
}

void reset_state()
{
	*gpio_swporta_dr &= ~(1U << HPS_LED_PIN);
}


int read_state()
{
	if (*gpio_ext_porta & HPS_KEY_MASK)
	{
    	return 1;
    }
    else
    {
    	return 0;
    }
}

void configure_dir()
{
    *gpio_swporta_ddr |=  (1 << HPS_LED_PIN);  
    *gpio_swporta_ddr &= ~(1 << HPS_KEY_PIN);
}

void* button_thr_func(void* ptr)
{
	int prev_state = (1U); // Default HIGH
	
    while(1)
    {
    	
    	
        int curr_state = read_state();
        
        if((curr_state == 0) && (prev_state != 0))
        {
        	pthread_mutex_lock(&mutex);
        	led_on = ~led_on;
        	pthread_mutex_unlock(&mutex);
        }
		
		prev_state = curr_state;
    }
}

void* led_thr_func(void* ptr)
{
    while(1)
    {
        if(led_on)
        {
            set_state();
            
            usleep(500000);
            
            reset_state();
            
            usleep(500000);
        }
        else
        {
            reset_state();
        }
    }
}


int main(void)
{
    void* virtual_base;
    int fd;
    
    pthread_t button_thread, led_thread;
    int iret1, iret2;
    
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

	// Get the address that maps to the direction register
	gpio_swporta_ddr = (unsigned int *)(virtual_base + GPIO1_REGS_DIR_OFFSET);
	
	// Get the address that maps to the data register
	gpio_swporta_dr  = (unsigned int *)(virtual_base + GPIO1_REGS_DATA_OFFSET);
	
	// Get the address that maps to the input data reading register
	gpio_ext_porta   = (unsigned int *)(virtual_base + GPIO1_REGS_EXT_PORTA_OFFSET);
	
    
    configure_dir();
    
    iret1 = pthread_create(&button_thread, NULL, button_thr_func, NULL);
    iret2 = pthread_create(&led_thread, NULL, led_thr_func, NULL);
    
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

