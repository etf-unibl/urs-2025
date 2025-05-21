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
#define GPIO1_REGS_INPUT_OFFSET (0x50)

#define HPS_BUTTON_PIN      (25U)
#define HPS_LED_PIN	        (24U)

volatile unsigned int blinking = 0;
volatile int previous_button_state=1;
volatile int current_button_state;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

int read_button_state(unsigned int* hps_input_port)
{
    unsigned int state = *hps_input_port >> 25;

    if(state%2==0) return 1;
    else return 0;

}

void* thread1_function(unsigned int* hps_input_port)
{
    while(1)
    {

        pthread_mutex_lock( &mutex1 );
        current_button_state = read_button_state(hps_input_port);

        //printf("%d\n",current_button_state);

        if(current_button_state==0 && previous_button_state==1)
        {
            previous_button_state = current_button_state;
            printf("change\n");
            
            blinking = ~blinking;         
                           
        }
        else if (previous_button_state==0 && current_button_state==1)   previous_button_state=current_button_state;
        else{
            printf("NOTCHANGED\n");
        }        

        pthread_mutex_unlock( &mutex1 );
        usleep(200000);
    }
}

void* thread2_function(unsigned int* led_control_addr)
{
    while(1)
    {        
        
        if(blinking)
        {
            printf("%u",blinking);
            pthread_mutex_lock( &mutex1 );
            *led_control_addr = *led_control_addr | (1 << HPS_LED_PIN);
            pthread_mutex_unlock( &mutex1 );
            usleep(200000);
            pthread_mutex_lock( &mutex1 );
            *led_control_addr = *led_control_addr & (0 << HPS_LED_PIN);
            pthread_mutex_unlock( &mutex1 );
            usleep(200000);
        }      
    }
}

int main(void)
{
    pthread_t thread1, thread2;
    volatile unsigned int* hps_led_dir_addr = NULL;
    volatile unsigned int* hps_led_port_addr = NULL;
    volatile unsigned int* hps_button_input_port_addr = NULL;
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
    // Get the address that maps to the input port register
    hps_button_input_port_addr = (unsigned int *)(virtual_base + GPIO1_REGS_INPUT_OFFSET);
    
    int iret1 = pthread_create( &thread1, NULL, thread1_function, hps_button_input_port_addr);
    int iret2 = pthread_create( &thread2, NULL, thread2_function, hps_led_port_addr);


    // Set HPS_LED direction to be output
    
    *hps_led_dir_addr &= (0 << HPS_BUTTON_PIN);
    *hps_led_dir_addr |= (1 << HPS_LED_PIN);

    // set hps_button direction as input: zero by default
    
    
    // Turn the HPS_LED off initially

    *hps_led_port_addr = *hps_led_port_addr & (0 << HPS_LED_PIN);


    pthread_join( thread1, NULL);
    pthread_join( thread2, NULL);

    while(1);
    

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

