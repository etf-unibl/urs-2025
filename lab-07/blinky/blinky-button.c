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
#include <stdbool.h>

#define GPIO1_REGS_BASE         (0xff709000)
#define GPIO1_REGS_SIZE         (0x00001000)
#define GPIO1_REGS_DATA_OFFSET  (0x0)
#define GPIO1_REGS_IN_OFFSET  (0x50)
#define GPIO1_REGS_DIR_OFFSET   (0x4)

#define HPS_LED_PIN	        (24U)
#define HPS_KEY_PIN	        (25U)

void* read_button(void * args);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool isBlinkingEnabled = true;

    volatile unsigned int* hps_led_dir_addr = NULL;
    volatile unsigned int* hps_led_port_addr = NULL;
    volatile unsigned int* hps_button_port_addr = NULL;
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
    hps_button_port_addr = (unsigned int *)(virtual_base + GPIO1_REGS_IN_OFFSET);
    
    // Set HPS_LED direction to be output
    /* TODO : Add your code here */
    *hps_led_dir_addr = (1 << HPS_LED_PIN);

    // SET HPS_KEY direction to be intput
    *hps_led_dir_addr &= ~(1 << HPS_KEY_PIN);
    
    // Turn the HPS_LED off initially
    /* TODO : Add your code here */
*hps_led_port_addr = (1 << HPS_LED_PIN);

	pthread_t buttonThread;
	if(pthread_create(&buttonThread, NULL, read_button, NULL)){
		printf("Thread creation failed \n");
		return 1;
	}
	
    // Loop infinitely and toggle the HPS_LED every 0.5 seconds
    while(1)
    {
	    if(isBlinkingEnabled){
	    pthread_mutex_lock(&mutex);
		*hps_led_port_addr |= (1 << HPS_LED_PIN	);
	    pthread_mutex_unlock(&mutex);
		usleep(500000);
	    pthread_mutex_lock(&mutex);
		*hps_led_port_addr &= ~(1 << HPS_LED_PIN);
	    pthread_mutex_unlock(&mutex);
		usleep(500000);
	    }
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

void* read_button(void * args){
	int prevState = 1;
	while(1){
		pthread_mutex_lock(&mutex);
		int state = (*hps_button_port_addr & (1 << HPS_KEY_PIN)) ? 1 : 0;
		if(prevState && !state){
		isBlinkingEnabled = !isBlinkingEnabled;
		prevState = 0;
		} 
		else if(state && !prevState){
		prevState = 1;
		}
		pthread_mutex_unlock(&mutex);
		usleep(2000);
	}	
}

