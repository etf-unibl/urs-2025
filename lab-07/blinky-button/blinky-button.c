#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

#define GPIO1_REGS_BASE (0xff709000)
#define GPIO1_REGS_SIZE (0x00001000)
#define GPIO1_REGS_DATA_OFFSET (0x0)
#define GPIO1_REGS_DIR_OFFSET (0x4)
#define GPIO1_REGS_EXT_OFFSET  (0x50)           // This GPIO register is used to read input data. (found in Cyclone V Register Map)

#define HPS_LED_PIN (24U)
#define HPS_KEY_PIN (25U)

volatile int blink_enabled = 1;
pthread_mutex_t gpio_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile unsigned int *gpio1_ext_port_addr = NULL;
volatile unsigned int *gpio1_port_addr = NULL;

void *button_thread_func(void *arg)
{
    int prev_state = 1;

    while (1)
    {
        // Lock monitor
        pthread_mutex_lock(&gpio_mutex);
        // read button state
        int current_state = (*gpio1_ext_port_addr & (1 << HPS_KEY_PIN)) ? 1 : 0;
        // unlock
        pthread_mutex_unlock(&gpio_mutex);

        if (prev_state == 1 && current_state == 0)
        {
            printf("State change detected.\n");
            blink_enabled = !blink_enabled; // Change state
            usleep(200000);                 // Debounce
        }
        prev_state = current_state;
        usleep(10000);
    }
    return NULL;
}

void *led_thread_func(void *arg)
{
    while (1)
    {
        if (blink_enabled)
        {
            // Lock monitor
            pthread_mutex_lock(&gpio_mutex);
            *gpio1_port_addr ^= (1 << HPS_LED_PIN); // Toggle LED
            // Unlock
            pthread_mutex_unlock(&gpio_mutex);

            usleep(500000);
        }
        else
        {
            // Ensure LED is off if blink is not enabled
            pthread_mutex_lock(&gpio_mutex);
            *gpio1_port_addr &= ~(1 << HPS_LED_PIN);
            pthread_mutex_unlock(&gpio_mutex);

            usleep(100000);
        }
    }
    return NULL;
}

int main(void)
{
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

    volatile unsigned int *gpio1_dir_addr = (unsigned int *)(virtual_base + GPIO1_REGS_DIR_OFFSET);
    gpio1_port_addr = (unsigned int *)(virtual_base + GPIO1_REGS_DATA_OFFSET);
    gpio1_ext_port_addr = (unsigned int *)(virtual_base + GPIO1_REGS_EXT_OFFSET);

    // Configure LED and button directions
    *gpio1_dir_addr |= (1 << HPS_LED_PIN);  // output
    *gpio1_dir_addr &= ~(1 << HPS_KEY_PIN); // input

    // Instantiate threads
    pthread_t thread_led, thread_button;
    pthread_create(&thread_led, NULL, led_thread_func, NULL);
    pthread_create(&thread_button, NULL, button_thread_func, NULL);

    // Join on threads
    pthread_join(thread_led, NULL);
    pthread_join(thread_button, NULL);

    // Destroy mutex (as recommended)
    pthread_mutex_destroy(&gpio_mutex);

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
