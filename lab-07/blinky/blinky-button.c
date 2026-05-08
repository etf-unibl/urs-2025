#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

#define GPIO1_REGS_BASE        (0xFF709000UL)
#define GPIO1_REGS_SIZE        (0x00001000UL)
#define GPIO1_REGS_DATA_OFFSET (0x00UL)
#define GPIO1_REGS_DIR_OFFSET  (0x04UL)
#define GPIO1_REGS_EXT_OFFSET  (0x50UL)

#define HPS_LED_PIN (24U)
#define HPS_KEY_PIN (25U)

//Resources shared between the threads
static volatile int      blinking = 1;
static pthread_mutex_t   lock     = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t    cond     = PTHREAD_COND_INITIALIZER;

// Register pointers
static volatile unsigned int *gpio_data = NULL;
static volatile unsigned int *gpio_dir  = NULL;
static volatile unsigned int *gpio_ext  = NULL;


//Helper functions for reading/writing to registers
static inline void led_write(unsigned int state)
{
    if (0 == state)
        *gpio_data &= ~(1U << HPS_LED_PIN);
    else
        *gpio_data |=  (1U << HPS_LED_PIN);
}

static inline unsigned int key_read(void)
{
    return (*gpio_ext >> HPS_KEY_PIN) & 1U;
}

// Button thread
static void *button_thread(void *arg)
{
    (void)arg;

    unsigned int key_prev    = 1;
    unsigned int key_current = 1;

    while (1)
    {
        key_current = key_read();

		// Button pressed
        if (1 == key_prev && 0 == key_current)
        {
            pthread_mutex_lock(&lock);
            blinking ^= 1;
            pthread_cond_signal(&cond);   // wake up the LED thread
            pthread_mutex_unlock(&lock);
        }

        key_prev = key_current;
        usleep(10000);  
    }

    return NULL;
}

//LED thread
static void *led_thread(void *arg)
{
    (void)arg;

    unsigned int led_state = 0;

	while (1)
	{
		pthread_mutex_lock(&lock);
		if (0 == blinking)         
		{
			led_write(0);          // reset and turn off once
			led_state = 0;         
		}
		while (0 == blinking)   // check if blinking off and should sleep
			pthread_cond_wait(&cond, &lock);
		pthread_mutex_unlock(&lock);
	
	
		led_write(led_state);
		led_state ^= 1U;
		usleep(500000); //500ms
	}

    return NULL;
}


int main(void)
{
    void      *virtual_base;
    int        fd;
    pthread_t  tid_button, tid_led;

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (-1 == fd)
    {
        printf("Error when opening /dev/mem! Exiting...\n");
        return 1;
    }

    virtual_base = mmap(NULL, GPIO1_REGS_SIZE,
                        PROT_READ | PROT_WRITE, MAP_SHARED,
                        fd, GPIO1_REGS_BASE);
    if (MAP_FAILED == virtual_base)
    {
        printf("Error when trying to mmap virtual to physical GPIO address! Exiting...\n");
        close(fd);
        return 1;
    }

    gpio_data = (volatile unsigned int *)((char *)virtual_base + GPIO1_REGS_DATA_OFFSET);
    gpio_dir  = (volatile unsigned int *)((char *)virtual_base + GPIO1_REGS_DIR_OFFSET);
    gpio_ext  = (volatile unsigned int *)((char *)virtual_base + GPIO1_REGS_EXT_OFFSET);

    // LED pin 24 output, KEY pin 25 input
    *gpio_dir |=  (1U << HPS_LED_PIN);
    *gpio_dir &= ~(1U << HPS_KEY_PIN);

    // LED starts as off
    led_write(0);

    // Start the threads
    if (0 != pthread_create(&tid_button, NULL, button_thread, NULL))
    {
        printf("Error when creating button_thread!\n");
        munmap(virtual_base, GPIO1_REGS_SIZE);
        close(fd);
        return 1;
    }

    if (0 != pthread_create(&tid_led, NULL, led_thread, NULL))
    {
        printf("Error when creating led_thread!\n");
        munmap(virtual_base, GPIO1_REGS_SIZE);
        close(fd);
        return 1;
    }
	// Main waits for the threads to finish (so the threads can stay alive)
    pthread_join(tid_button, NULL);
    pthread_join(tid_led,    NULL);

    
    // Unmap previously mapped virtual address space
    if (0 != munmap(virtual_base, GPIO1_REGS_SIZE))
    {
        printf("Error when trying to munmap previously mapped addresses! Exiting...\n");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}