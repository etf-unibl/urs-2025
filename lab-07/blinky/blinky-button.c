/*
 * blinky-button.c (Lab 7 - user-space device control via /dev/mem)
 *
 * - HPS GPIO1 base: 0xff709000
 * - LED:  GPIO1 Port A bit 24 (write via SWPORTA_DR)
 * - KEY:  GPIO1 Port A bit 25 (read via EXT_PORTA), active-low
 *
 * Build:
 *   arm-linux-gcc -O2 -Wall -pthread -o blinky-button blinky-button.c
 *
 * Run (as root):
 *   ./blinky-button
 * Stop:
 *   Ctrl+C
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <pthread.h>
#include <signal.h>

#define GPIO1_REGS_BASE   (0xff709000)
#define GPIO1_REGS_SIZE   (0x00001000)

/* DesignWare APB GPIO (Port A) offsets */
#define GPIO_SWPORTA_DR   (0x00)  /* data register (read/write) */
#define GPIO_SWPORTA_DDR  (0x04)  /* direction: 1=output, 0=input */
#define GPIO_EXT_PORTA    (0x50)  /* external port (read pin state) */

#define HPS_LED_PIN       (24U)
#define HPS_KEY_PIN       (25U)

#define BIT(n)            (1U << (n))

static volatile uint32_t *gpio_dr  = NULL;
static volatile uint32_t *gpio_ddr = NULL;
static volatile uint32_t *gpio_ext = NULL;

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int blink_enabled = 1;

static volatile sig_atomic_t keep_running = 1;

static void on_sigint(int sig)
{
    (void)sig;
    keep_running = 0;
}

/* LED helpers */
static inline void led_off(void)
{
    *gpio_dr &= ~BIT(HPS_LED_PIN);
}

static inline void led_toggle(void)
{
    *gpio_dr ^= BIT(HPS_LED_PIN);
}

/* KEY read (active-low) */
static inline int key_is_pressed(void)
{
    uint32_t v = *gpio_ext;                 /* read physical pin level */
    return (v & BIT(HPS_KEY_PIN)) == 0;     /* pressed => 0 */
}

static void *blink_thread(void *arg)
{
    (void)arg;

    while (keep_running) {
        int en;

        pthread_mutex_lock(&lock);
        en = blink_enabled;
        pthread_mutex_unlock(&lock);

        if (en) {
            led_toggle();
            usleep(500000); /* 0.5 s */
        } else {
            led_off();
            usleep(50000);  /* 50 ms */
        }
    }

    led_off();
    return NULL;
}

static void *button_thread(void *arg)
{
    (void)arg;

    int last_pressed = 0;

    while (keep_running) {
        int pressed = key_is_pressed();

        /* detect new press (0 -> 1 in "pressed" state) */
        if (pressed && !last_pressed) {
            /* debounce */
            usleep(30000); /* 30 ms */
            if (key_is_pressed()) {
                pthread_mutex_lock(&lock);
                blink_enabled = !blink_enabled;
                int en = blink_enabled;
                pthread_mutex_unlock(&lock);

                printf("HPS_KEY press detected -> blink_enabled=%d\n", en);
                fflush(stdout);

                /* wait until release to avoid repeated toggles while holding */
                while (keep_running && key_is_pressed()) {
                    usleep(10000);
                }
            }
        }

        last_pressed = pressed;
        usleep(10000); /* polling period 10 ms */
    }

    return NULL;
}

int main(void)
{
    signal(SIGINT, on_sigint);

    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open(/dev/mem)");
        return 1;
    }

    void *virtual_base = mmap(NULL, GPIO1_REGS_SIZE,
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED, fd, GPIO1_REGS_BASE);
    if (virtual_base == MAP_FAILED) {
        perror("mmap(GPIO1)");
        close(fd);
        return 1;
    }

    gpio_dr  = (volatile uint32_t *)((char *)virtual_base + GPIO_SWPORTA_DR);
    gpio_ddr = (volatile uint32_t *)((char *)virtual_base + GPIO_SWPORTA_DDR);
    gpio_ext = (volatile uint32_t *)((char *)virtual_base + GPIO_EXT_PORTA);

    /* Configure directions:
       LED output, KEY input */
    *gpio_ddr |= BIT(HPS_LED_PIN);
    *gpio_ddr &= ~BIT(HPS_KEY_PIN);

    led_off();

    printf("blinky-button running.\n");
    printf("LED: GPIO1.A[%u] via DR, KEY: GPIO1.A[%u] via EXT_PORTA (active-low)\n",
           HPS_LED_PIN, HPS_KEY_PIN);
    printf("Press USER BUTTON (HPS_KEY) to toggle blinking. Ctrl+C to exit.\n");

    pthread_t t1, t2;
    if (pthread_create(&t1, NULL, blink_thread, NULL) != 0) {
        perror("pthread_create(blink_thread)");
        munmap(virtual_base, GPIO1_REGS_SIZE);
        close(fd);
        return 1;
    }

    if (pthread_create(&t2, NULL, button_thread, NULL) != 0) {
        perror("pthread_create(button_thread)");
        keep_running = 0;
        pthread_join(t1, NULL);
        munmap(virtual_base, GPIO1_REGS_SIZE);
        close(fd);
        return 1;
    }

    pthread_join(t2, NULL);
    pthread_join(t1, NULL);

    munmap(virtual_base, GPIO1_REGS_SIZE);
    close(fd);
    return 0;
}