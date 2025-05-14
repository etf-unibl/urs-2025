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

/** Physic Base Address of GPIO1 Register */
#define GPIO1_REGS_BASE             (0xff709000)

/** Size of GPIO1 Register */
#define GPIO1_REGS_SIZE             (0x00001000)

/** Offset of Port A Data Direction Register */
#define DATA_DIRECTION_REG_OFFSET   (0x4)

/** Offset of Port A Data Register */
#define DATA_REG_OFFSET             (0x0)

#define HPS_LED_PIN	                (24U)
#define HPS_KEY_PIN                 (21U)

int main(void)
{
    int fd = open("/dev/mem", (O_RDWR | O_SYNC));

    if (-1 == fd)
    {
        printf("Error when opening /dev/mem! Exiting...\n");
        return 1;
    }

    /*
     * void *mmap(void addr[.length], size_t length, int prot, int flags, int fd, off_t offset)
     *
     *      \param addr[.length] : ostavljamo kernelu da odluci od koje virtuelne adrese ce biti mapirano dato podrucje
     *      \param length        : GPIO1_REGS_SIZE je velicina adresnog prostora kog mapiramo
     *      \param prot          : nivo zastite mapiranog memorijskog podrucja mora biti uskladjen sa flegovima koristenim za otvaranje /dev/mem fajla
     *      \param flags         : flegovi za specificiranje nacina mapiranja i pristupa mapiranom podrucju od strane drugih procesa
     *      \param fd            : fajl deskriptor prethodno otvorenog virtuelnog uređaja (/dev/mem)
     *      \param offset        : bazna adresa memorijskog podrucja u kojem se nalaze registri uredjaja
     *
     *      \ret   void*         : funkcija vraca virtuelnu adresu koja je mapirana sa baznom adresom uredjaja u fizickom adresnom prostoru
     */
    void* virtual_base = mmap(NULL, GPIO1_REGS_SIZE, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, GPIO1_REGS_BASE);

    if (MAP_FAILED == virtual_base)
    {
        printf("Error when trying to mmap virtual to physical GPIO address! Exiting...\n");
        close(fd);
        return 1;
    }

    // Virtual address that maps to the data direction register
    volatile unsigned int*  gpio_swporta_ddr = (unsigned int *)(virtual_base + DATA_DIRECTION_REG_OFFSET);

    // Virtual address that maps to the data register
    volatile unsigned int*  gpio_swporta_dr  = (unsigned int *)(virtual_base + DATA_REG_OFFSET);
    
    /* ------------------------------------------------------------------------------------------------------ */
    /*                                              HPS KEY & HPS LED                                         */
    /* ------------------------------------------------------------------------------------------------------ */

    /* Set HPS_LED direction to be output */
    *(gpio_swporta_ddr) |= 1u << HPS_LED_PIN;
    /* Set HPS_KEY direction to be intput */
    *(gpio_swporta_ddr) &= ~(1u << HPS_KEY_PIN);
    
    /* Turn the HPS_LED off initially */
    *(gpio_swporta_dr) &= ~(1 << HPS_LED_PIN);

    while(1)
    {
        usleep(500000);
        *gpio_swporta_dr ^= (1 << HPS_LED_PIN);
    }

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

