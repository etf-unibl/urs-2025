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
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

#define GPIO1_REGS_BASE (0xff709000)
#define GPIO1_REGS_SIZE (0x00001000)

#define GPIO_SWPORTA_DR 0x00
#define GPIO_SWPORTA_DDR 0x04
#define GPIO_EXT_PORTA 0x50

#define GPIO1_REGS_DATA_OFFSET (0x0)
#define GPIO1_REGS_DIR_OFFSET (0x4)

#define HPS_LED_PIN (24U)

#define LED_BIT (1 << 24)
#define KEY_BIT (1 << 25)

volatile unsigned int *gpio_dr;
volatile unsigned int *gpio_ext;
int blinking_enabled = 1;

void *key_thread_func(void *arg)
{
    int last_state = 1;

    while (1)
    {
        int current_state = (*gpio_ext & KEY_BIT) >> 25;

        if (last_state == 1 && current_state == 0)
        {
            blinking_enabled = !blinking_enabled;
            printf("Button pressed! Blinking %s\n", blinking_enabled ? "enabled" : "disabled");
            usleep(200000);
        }
        last_state = current_state;
        usleep(10000);
    }
    return NULL;
}

void *led_thread_func(void *arg)
{
    while (1)
    {
        if (blinking_enabled)
        {
            *gpio_dr ^= LED_BIT;
            usleep(500000);
        }
        else
        {
            *gpio_dr &= ~LED_BIT;
            usleep(100000);
        }
    }
    return NULL;
}

int main(void)
{
    volatile unsigned int *hps_led_dir_addr = NULL;
    volatile unsigned int *hps_led_port_addr = NULL;
    void *virtual_base;
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
    gpio_dr = (unsigned int *)(virtual_base + GPIO_SWPORTA_DR);
    // Get the address that maps to the HPS_LED port register
    gpio_ext = (unsigned int *)(virtual_base + GPIO_EXT_PORTA);

    volatile unsigned int *gpio_ddr = (unsigned int *)(virtual_base + GPIO_SWPORTA_DDR);

    *gpio_ddr |= LED_BIT;
    *gpio_ddr &= ~KEY_BIT;

    pthread_t key_tid, led_tid;
    pthread_create(&key_tid, NULL, key_thread_func, NULL);
    pthread_create(&led_tid, NULL, led_thread_func, NULL);

    pthread_join(key_tid, NULL);
    pthread_join(led_tid, NULL);

    // Unmap previously mapped virtual address space
    if (0 != munmap(virtual_base, GPIO1_REGS_SIZE))
    {
        printf("Error when trying to munmap previously mapped addresses! Exiting...\n");
        close(fd);
        return 1;
    }

    // Close the file descriptor
    close(fd);

    return 0;
}
