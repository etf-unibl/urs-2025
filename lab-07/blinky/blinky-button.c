#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdint.h>

#define GPIO1_REGS_BASE         (0xff709000)
#define GPIO1_REGS_SIZE         (0x00001000)
#define GPIO1_DATA_OFFSET       (0x0)
#define GPIO1_DIR_OFFSET        (0x4)
#define EXT_OFFSET            (0x50)

#define HPS_LED_PIN             (24U)
#define HPS_KEY_PIN             (25U) //  pin 25  button

volatile unsigned int* gpio_dir = NULL;
volatile unsigned int* gpio_data = NULL;
volatile unsigned int* gpio_ext_porta = NULL;
pthread_mutex_t led_mutex;
int led_blink_on = 1;

void* led_thread(void* arg)
{
    while (1)
    {
        pthread_mutex_lock(&led_mutex);
        int blink = led_blink_on;
        pthread_mutex_unlock(&led_mutex);

        if (blink)
        {
            *gpio_data ^= (1 << HPS_LED_PIN); // toggle LED
        }

        usleep(500000); // 0.5 sekunde
    }
    return NULL;
}

void* button_thread(void* arg)
{
    int prev_state = (*gpio_ext_porta & (1 << HPS_KEY_PIN)) ? 1 : 0;
    while (1)
    {
        int current_state = (*gpio_ext_porta & (1 << HPS_KEY_PIN)) ? 1 : 0;

        if (prev_state == 1 && current_state == 0) // falling edge
        {
		 printf("Taster pritisnut!\n");
            pthread_mutex_lock(&led_mutex);
            led_blink_on = !led_blink_on;
            pthread_mutex_unlock(&led_mutex);
        }

        prev_state = current_state;
        usleep(50000); // debounce delay: 50ms
    }
    return NULL;
}

int main(void)
{
    int fd;
    void* virtual_base;

    fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd == -1)
    {
        printf("Error opening /dev/mem.\n");
        return 1;
    }

    virtual_base = mmap(NULL, GPIO1_REGS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO1_REGS_BASE);
    if (virtual_base == MAP_FAILED)
    {
        printf("mmap failed.\n");
        close(fd);
        return 1;
    }

    gpio_dir = (unsigned int*)(virtual_base + GPIO1_DIR_OFFSET);
    gpio_data = (unsigned int*)(virtual_base + GPIO1_DATA_OFFSET);
	gpio_ext_porta =  (unsigned int*)(virtual_base + EXT_OFFSET);
    // LED output, KEY input
    *gpio_dir |= (1 << HPS_LED_PIN);   // LED kao izlaz
    *gpio_dir &= ~(1 << HPS_KEY_PIN);  // taster kao ulaz
    *gpio_data |= (1 << HPS_LED_PIN); // LED off

    pthread_mutex_init(&led_mutex, NULL);

    pthread_t led_tid, button_tid;

    pthread_create(&led_tid, NULL, led_thread, NULL);
    pthread_create(&button_tid, NULL, button_thread, NULL);

    // Glavna nit čeka završetak pod-niti, u ovom slucaju  se nikad ne dešava
    pthread_join(led_tid, NULL);
    pthread_join(button_tid, NULL);

    munmap(virtual_base, GPIO1_REGS_SIZE);
    close(fd);
    pthread_mutex_destroy(&led_mutex);
    return 0;
}

