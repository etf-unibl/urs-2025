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
#define EXT_OFFSET		(0x50)

// Bit position of the HPS LED pin in the GPIO1 module
#define HPS_LED_PIN	        (24U)

// Bit position of the HPS KEY pin in the GPIO1 module
#define HPS_KEY_PIN         (25U)


volatile unsigned int* hps_led_dir_addr = NULL;
volatile unsigned int* hps_led_port_addr = NULL;
volatile unsigned int* gpio_ext_porta = NULL;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int blinking_enabled = 1;
volatile int running = 1;

//LED thread
void* led_thred_func(void* arg){
    while(running){
        pthread_mutex_lock(&mutex);
        int local_blink = blinking_enabled;
        pthread_mutex_unlock(&mutex);
       
        if(local_blink){
            *hps_led_port_addr ^= (1 << HPS_LED_PIN);
        }
        usleep(500000);
    }
    return NULL;
}
void* button_thread_func(void* arg){
    int prev_key_state = (*gpio_ext_porta & (1 << HPS_KEY_PIN))?1:0;
    while(running){
        int current_key_state = (*gpio_ext_porta & (1 << HPS_KEY_PIN)) ? 1 : 0;

        if(!current_key_state && prev_key_state){
            pthread_mutex_lock(&mutex);
            blinking_enabled=!blinking_enabled;
            pthread_mutex_unlock(&mutex);
        }
        prev_key_state = current_key_state;
        usleep(100000); //debounce
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
    hps_led_dir_addr = (unsigned int *)(virtual_base + GPIO1_REGS_DIR_OFFSET);
    // Get the address that maps to the HPS_LED port register
    hps_led_port_addr = (unsigned int *)(virtual_base + GPIO1_REGS_DATA_OFFSET);
    gpio_ext_porta = (unsigned int*)(virtual_base + EXT_OFFSET);
    // Set HPS_LED direction to be output
    *hps_led_dir_addr |= (1 << HPS_LED_PIN);
    // Set HPS_KEY direction to be input
    *hps_led_dir_addr &= ~(1 << HPS_KEY_PIN);

    // Turn the HPS_LED off initially
    *hps_led_port_addr |= ~(1<< HPS_LED_PIN);

    // Loop infinitely and toggle the HPS_LED every 0.5 seconds
    pthread_t led_thread, button_thread; 

    int iret_led = pthread_create(&led_thread, NULL, led_thred_func, NULL);
    int iret_button = pthread_create(&button_thread, NULL, button_thread_func, NULL);

    pthread_join(led_thread, NULL);
    pthread_join(button_thread, NULL);

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

