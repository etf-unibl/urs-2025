/*
*******************************************************************************
Name 		: blinky.c
Author 		: Nikola Stankovic
Version 	: 0.2
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
#define GPIO1_REGS_EXT_OFFSET   (0x50)   // dodat registar za ulazne podatke

#define HPS_LED_PIN             (24U)
#define HPS_KEY_PIN             (25U)

volatile int blink_enabled = 1;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;  

/* LED nit – pali/gasi LED na osnovu blink_enabled */
void *led_thread(void *arg) {
    volatile unsigned int *port = (volatile unsigned int *)arg;   // DATA registar
    while (1) {
        pthread_mutex_lock(&lock);
        int enable = blink_enabled;
        pthread_mutex_unlock(&lock);

        static int cnt = 0;
        if (++cnt % 100 == 0) {
            printf("LED thread: enable=%d\n", enable);
            fflush(stdout);
        }

        if (enable) {
            *port ^= (1 << HPS_LED_PIN);
            usleep(500000);
        } else {
            *port &= ~(1 << HPS_LED_PIN);
            usleep(100000);
        }
    }
    return NULL;
}

/* Taster nit – čita HPS_KEY iz EXT_PORTA i menja blink_enabled */
void *button_thread(void *arg) {
    volatile unsigned int *ext_port = (volatile unsigned int *)arg; // EXT_PORTA registar
    int prev_state = (*ext_port >> HPS_KEY_PIN) & 1;
    printf("Initial button state: %d\n", prev_state);
    fflush(stdout);

    while (1) {
        int current_state = (*ext_port >> HPS_KEY_PIN) & 1;

        static int counter = 0;
        if (++counter % 50 == 0) {
            //printf("BTN state: %d, blink_enabled: %d\n", current_state, blink_enabled);
            fflush(stdout);
        }

        if (prev_state == 1 && current_state == 0) {
            //printf("Falling edge detected!\n"); fflush(stdout);
            usleep(20000);   // debounce
            if (((*ext_port >> HPS_KEY_PIN) & 1) == 0) {
                pthread_mutex_lock(&lock);
                blink_enabled = !blink_enabled;
                pthread_mutex_unlock(&lock);
                printf("blink_enabled toggled to: %d\n", blink_enabled); fflush(stdout);
                while (((*ext_port >> HPS_KEY_PIN) & 1) == 0) {
                    usleep(10000);
                }
                //printf("Button released.\n"); fflush(stdout);
            }
        }
        prev_state = current_state;
        usleep(10000);
    }
    return NULL;
}

int main(void) {
    volatile unsigned int *hps_led_dir_addr = NULL;
    volatile unsigned int *hps_led_port_addr = NULL;
    volatile unsigned int *hps_ext_port_addr = NULL;  // novi pokazivač za EXT_PORTA
    void *virtual_base;
    int fd;

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        printf("Error opening /dev/mem\n");
        return 1;
    }

    virtual_base = mmap(NULL, GPIO1_REGS_SIZE, PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, GPIO1_REGS_BASE);
    if (virtual_base == MAP_FAILED) {
        printf("mmap failed\n");
        close(fd);
        return 1;
    }

    hps_led_dir_addr = (unsigned int *)(virtual_base + GPIO1_REGS_DIR_OFFSET);
    hps_led_port_addr = (unsigned int *)(virtual_base + GPIO1_REGS_DATA_OFFSET);
    hps_ext_port_addr = (unsigned int *)(virtual_base + GPIO1_REGS_EXT_OFFSET); // EXT_PORTA

    // konfiguracija pinova
    *hps_led_dir_addr |= (1 << HPS_LED_PIN);   // LED izlaz
    *hps_led_dir_addr &= ~(1 << HPS_KEY_PIN);  // KEY ulaz

    // početno isključi LED
    *hps_led_port_addr &= ~(1 << HPS_LED_PIN);

    pthread_t led_tid, button_tid;
    // LED nit koristi DATA registar (kao i ranije)
    pthread_create(&led_tid, NULL, led_thread, (void *)hps_led_port_addr);
    // Taster nit sada koristi EXT_PORTA registar
    pthread_create(&button_tid, NULL, button_thread, (void *)hps_ext_port_addr);

    pthread_join(led_tid, NULL);
    pthread_join(button_tid, NULL);

    munmap(virtual_base, GPIO1_REGS_SIZE);
    close(fd);
    return 0;
}