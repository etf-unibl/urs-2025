/*
*******************************************************************************
Name 		: blinky-button.c
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

#define HPS_KEZ_PIN (25U)

int blink_enabled = 1;
pthread_mutex_t lock;

#define GPIO1_REGS_BASE         (0xff709000)
#define GPIO1_REGS_SIZE         (0x00001000)
#define GPIO1_REGS_DATA_OFFSET  (0x0)
#define GPIO1_REGS_DIR_OFFSET   (0x4)
#define GPIO1_REGS_EXT_PORT_OFFSET   (0x50)

#define HPS_LED_PIN	        (24U)

void *button_thread_function(void *args);
void *led_thread_function(void *args);

void *led_thread_function(void *args){

	volatile unsigned int *port_addr = (volatile unsigned int *)args;
	while(1){
		pthread_mutex_lock(&lock);
		int run = blink_enabled;
		pthread_mutex_unlock(&lock);
		
		if(run){
			*port_addr ^= (1 << HPS_LED_PIN);
		}else{
			*port_addr &= ~(1 << HPS_LED_PIN);
		}
		
		usleep(200000);
		usleep(10000);

	}
	return NULL;

}

void *button_thread_function(void *args){

	volatile unsigned int *port_addr = (volatile unsigned int *)args;
	int last_state = (*port_addr >> HPS_KEZ_PIN) & 1;

	while(1){

	int current_state = (*port_addr >> HPS_KEZ_PIN) & 1;
		printf("stanje pina %d\n", current_state);

	if (last_state == 1 && current_state == 0){
		pthread_mutex_lock(&lock);
		blink_enabled = !blink_enabled;
		pthread_mutex_unlock(&lock);
		printf("Pritisnut taster...%s\n", blink_enabled ? "ON" : "OFF");
		usleep(300000);
	}

	last_state = current_state;
	usleep(20000);

	}
	return NULL;

}
int main(void)
{

    pthread_t button_id, led_id;


    volatile unsigned int* hps_led_dir_addr = NULL;
    volatile unsigned int* hps_led_port_addr = NULL;
    volatile unsigned int* hps_key_port_addr = NULL;
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
    hps_key_port_addr = (unsigned int *)(virtual_base + GPIO1_REGS_EXT_PORT_OFFSET);
    // Set HPS_LED direction to be output
    /* TODO : Add your code here */
    *hps_led_dir_addr |= (1 << HPS_LED_PIN);
    
    // Turn the HPS_LED off initially
    /* TODO : Add your code here */
    *hps_led_dir_addr &= ~(1 << HPS_KEZ_PIN);


pthread_mutex_init(&lock,NULL);

pthread_create(&led_id, NULL, led_thread_function, (void *)hps_led_port_addr);
pthread_create(&button_id, NULL, button_thread_function, (void *)hps_key_port_addr);


pthread_join(led_id, NULL);
pthread_join(button_id, NULL);

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
