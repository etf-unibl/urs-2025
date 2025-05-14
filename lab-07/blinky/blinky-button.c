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
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>

#define GPIO1_REGS_BASE         (0xff709000)
#define GPIO1_REGS_SIZE         (0x00001000)
#define GPIO1_REGS_DATA_OFFSET  (0x0)
#define GPIO1_REGS_DIR_OFFSET   (0x4)
#define GPIO1_REGS_IN_OFFSET    (0X50)


#define HPS_LED_PIN	        (24U)
#define HPS_BUTTON_PIN          (25U)

pthread_mutex_t mode_lock = PTHREAD_MUTEX_INITIALIZER;
int blink_enabled = 1;

volatile uint32_t *hps_dir_addr = NULL;
volatile uint32_t *hps_led_port_addr = NULL;
volatile uint32_t *hps_button_port_addr = NULL;

void *button_thread(void *args){
	unsigned int prev_state = ((*hps_button_port_addr >> HPS_BUTTON_PIN) & 0x1);
        while(1){
	    unsigned int data = ((*hps_button_port_addr >> HPS_BUTTON_PIN) & 0x1);
	    if (data != prev_state && data == 0) {
            pthread_mutex_lock(&mode_lock);
            blink_enabled = !blink_enabled;
            pthread_mutex_unlock(&mode_lock);
            usleep(200000);
        }
        prev_state = data;
        usleep(10000);  
    }
    return NULL;
}

void *led_thread(void *arg) {
	int on;   
    while (1) {    
	pthread_mutex_lock(&mode_lock);
        on = blink_enabled;
        pthread_mutex_unlock(&mode_lock);

        if (on) {
            // toggle LED
            *hps_led_port_addr |= (1 << HPS_LED_PIN);
            usleep(500000);
            *hps_led_port_addr &= ~(1 << HPS_LED_PIN);
            usleep(500000);
        } else {
            *hps_led_port_addr &= ~(1 << HPS_LED_PIN);
            usleep(100000);
        }
    }
    return NULL;
}

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
    hps_dir_addr = (unsigned int *)(virtual_base + GPIO1_REGS_DIR_OFFSET);
    // Get the address that maps to the HPS_LED port register
    hps_led_port_addr = (unsigned int *)(virtual_base + GPIO1_REGS_DATA_OFFSET);
    hps_button_port_addr = (unsigned int *)(virtual_base + GPIO1_REGS_IN_OFFSET);

    *hps_dir_addr |= (1 << HPS_LED_PIN); //output
    *hps_dir_addr &= ~(1 << HPS_BUTTON_PIN); // BUTTON input
    // Turn the HPS_LED off initially
    *hps_led_port_addr &= ~(1 << HPS_LED_PIN); //off

   pthread_t th_button, th_led;
   if (pthread_create(&th_button, NULL, button_thread, NULL) != 0) {
        perror("pthread_create button");
        munmap(virtual_base, GPIO1_REGS_SIZE);
        close(fd);
        return EXIT_FAILURE;
   }
   if (pthread_create(&th_led, NULL, led_thread, NULL) != 0) {
        perror("pthread_create led");
        munmap(virtual_base, GPIO1_REGS_SIZE);
        close(fd);
        return EXIT_FAILURE;
   }

   pthread_join(th_button, NULL);
   pthread_join(th_led, NULL);

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

