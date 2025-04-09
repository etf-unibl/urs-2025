/*
*******************************************************************************
Name : startup.S
Author : Mladen Knezic
Version : 0.1
Copyright : BSD License
Description : Simple Cortex a9 startup rutine in assembler
*******************************************************************************
*/

_start:
    /* Initialize .data section */
    ldr r0, =__text_end__
    ldr r1, =__data_start__
    ldr r2, =__data_end__
    sub r3, r2, r1

    /* Handle the case when .data section size is 0 */
    cmp r3, #0
    beq init_bss

copy_data:
    ldrb r4, [r0], #1
    strb r4, [r1], #1
    subs r3, r3, #1
    bne copy_data

init_bss:
    /* Initialize .bss section  */
    ldr r0, =__bss_start__
    ldr r1, =__bss_end__
    sub r2, r1, r0

    /* Handle the case when .bss section size is 0 */
    cmp r2, #0
    beq init_stack
    mov r4, #0

zero_bss:
    strb r4, [r0], #1
    subs r2, r2, #1
    bne zero_bss

init_stack:
    ldr r1, =__stack_start__
    mov sp, r1
    bl main
    b .
