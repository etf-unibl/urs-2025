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
#define GPIO1_REGS_EXT_OFFSET   (0x50)

#define HPS_LED_PIN	        (24U)
#define HPS_KEY_PIN	        (25U)

volatile unsigned int* hps_led_dir_addr = NULL;
volatile unsigned int* hps_led_port_addr = NULL;
volatile unsigned int* hps_ext_port_addr = NULL;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
int blink_enabled = 1;

void *check_button();
void *toggle_led();

int main(void)
{
    void* virtual_base;
    int fd;

	pthread_t thread1, thread2;
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

    // Get the address that maps to the HPS_LED direction register
    hps_led_dir_addr = (unsigned int *)(virtual_base + GPIO1_REGS_DIR_OFFSET);
    // Get the address that maps to the HPS_LED port register
    hps_led_port_addr = (unsigned int *)(virtual_base + GPIO1_REGS_DATA_OFFSET);

    hps_ext_port_addr = (unsigned int *)(virtual_base + GPIO1_REGS_EXT_OFFSET);
    
	*hps_led_dir_addr &= ~(1 << HPS_KEY_PIN);
	*hps_led_dir_addr |= (1 << HPS_LED_PIN);
	*hps_led_port_addr &= ~(1 << HPS_LED_PIN);

	printf("%X\n", *hps_led_dir_addr);
	fflush(stdout);
    
	iret1 = pthread_create(&thread1, NULL, check_button, NULL);
	iret1 = pthread_create(&thread1, NULL, toggle_led, NULL);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

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

void *check_button() {
	while(1) {
		if((*hps_ext_port_addr & (1 << HPS_KEY_PIN)) != (1 << HPS_KEY_PIN)) {
			usleep(100000);
			if((*hps_ext_port_addr & (1 << HPS_KEY_PIN)) != (1 << HPS_KEY_PIN)) {

				pthread_mutex_lock(&mutex1);
					if(blink_enabled == 0)
						blink_enabled = 1;
					else blink_enabled = 0;
				pthread_mutex_unlock(&mutex1);
			}
		}
	}
}

void *toggle_led() {
	while(1) {
		if(blink_enabled) {
			*hps_led_port_addr ^= (1U << HPS_LED_PIN);
			usleep(500000);
		}
	}
}
