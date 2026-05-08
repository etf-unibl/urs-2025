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

#define GPIO1_REGS_BASE         (0xff709000)
#define GPIO1_REGS_SIZE         (0x00001000)
#define GPIO1_REGS_DATA_OFFSET  (0x0)
#define GPIO1_REGS_DIR_OFFSET   (0x4)

#define HPS_LED_PIN	        (24U)

int main(void)
{
    volatile unsigned int* hps_led_dir_addr = NULL;
    volatile unsigned int* hps_led_port_addr = NULL;
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
    
    // Set HPS_LED direction to be output
    *hps_led_dir_addr |= (1 << HPS_LED_PIN);        // The 24th bit represents direction of our pin.
    
    // Turn the HPS_LED off initially
    *hps_led_port_addr &= ~(1 << HPS_LED_PIN);      // Write LOW to 24th bit.

    int led_on = 0;

    // Loop infinitely and toggle the HPS_LED every 0.5 seconds
    while(1)
    {
        if(led_on) {
            *hps_led_port_addr &= ~(1 << HPS_LED_PIN);
            led_on = 0;
        } else {
            *hps_led_port_addr |= (1 << HPS_LED_PIN);
            led_on = 1;
        }

        usleep(500000);                 // usleep accepts microseconds as parameter
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

