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
	ldr     r1, =__stack_start__
	mov sp,r1
	bl main
	b .
