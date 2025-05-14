#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

/** Physic Base Address of GPIO1 Register */
#define GPIO1_REGS_BASE             (0xff709000)

/** Size of GPIO1 Register */
#define GPIO1_REGS_SIZE             (0x00001000)

/** Offset of Port A Data Direction Register */
#define DATA_DIRECTION_REG_OFFSET   (0x4)

/** Offset of Port A Data Register */
#define DATA_REG_OFFSET             (0x0)

/** GPIO register is used to read input data */
#define EXTERNAL_PORT_REG           (0x50)

#define HPS_LED_PIN	                (24U)
#define HPS_KEY_PIN                 (25U)

volatile unsigned int*  gpio_swporta_ddr = NULL;
volatile unsigned int*  gpio_swporta_dr  = NULL;
volatile unsigned int*  gpio_ext_porta   = NULL;

pthread_mutex_t blink_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile unsigned char blinking = 0xFF;

unsigned int read_input_data(unsigned int pin)
{
	// Set default to HIGH
	unsigned read_val = 1;

	// Read the specified GPIO pin state (register gpio_ext_porta)
	unsigned int value = (*gpio_ext_porta & (1u << pin));

	// If the bit on pin position is 0, then return LOW
	// Otherwise, we return HIGH, which is default
	if (0 == value)
	{
		read_val = 0;
	}

	return read_val;
}

void* key_thread(void* arg)
{
    unsigned int prev_key  = 1;

    while (1)
    {
        unsigned int current_key = read_input_data(HPS_KEY_PIN);

        if ( (current_key == 0) && (prev_key != 0) )
        {
            blinking = ~blinking;
            usleep(200000); // debounce delay
        }

        prev_key = current_key;

        usleep(100000);
    }
    return NULL;
}

void write_gpio(unsigned int pin, unsigned char value)
{
	// Access port data register (gpio_swporta_dr)
	if (0 == value)
	{
		// Set the bit on pin position to LOW
		*gpio_swporta_dr &= ~(1 << pin);
	}
	else
	{
		// Set the bit on pin position to HIGH
		*gpio_swporta_dr |= (1 << pin);
	}
}

void* led_thread(void* arg)
{
    unsigned char value = 0xFF;

    while (1)
    {    
        if (blinking)
        {
            usleep(500000);
            write_gpio(HPS_LED_PIN, value);
            value = ~value;
        }

        usleep(500000); // 0.5 sekundi delay za blink
    }
    return NULL;
}


int main(void)
{
    int fd = open("/dev/mem", (O_RDWR | O_SYNC));

    if (-1 == fd)
    {
        printf("Error when opening /dev/mem! Exiting...\n");
        return 1;
    }

    void* virtual_base = mmap(NULL, GPIO1_REGS_SIZE, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, GPIO1_REGS_BASE);

    if (MAP_FAILED == virtual_base)
    {
        printf("Error when trying to mmap virtual to physical GPIO address! Exiting...\n");
        close(fd);
        return 1;
    }

    // Virtual address that maps to the data direction register
    gpio_swporta_ddr = (unsigned int *)(virtual_base + DATA_DIRECTION_REG_OFFSET);

    // Virtual address that maps to the data register
    gpio_swporta_dr  = (unsigned int *)(virtual_base + DATA_REG_OFFSET);

    // Virtual address that maps to the register for input data reading
    gpio_ext_porta   = (unsigned int *)(virtual_base + EXTERNAL_PORT_REG);
    
    /* ------------------------------------------------------------------------------------------------------ */
    /*                                              HPS KEY & HPS LED                                         */
    /* ------------------------------------------------------------------------------------------------------ */

    /* Set HPS_LED direction to be output */
    *(gpio_swporta_ddr) |= 1u << HPS_LED_PIN;
    /* Set HPS_KEY direction to be intput */
    *(gpio_swporta_ddr) &= ~(1u << HPS_KEY_PIN);

    /* Turn the HPS_LED off initially */
    *(gpio_swporta_dr) &= ~(1 << HPS_LED_PIN);
    
    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, key_thread, NULL);
    pthread_create(&thread2, NULL, led_thread, NULL);

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

