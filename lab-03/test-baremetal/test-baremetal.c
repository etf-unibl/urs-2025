/**
*******************************************************************************
Name 		: test-baremetal.c
Author 		: Mladen Knezic
Version 	: 0.1
Copyright 	: BSD License
Description	: Simple Cortex A9 bare-metal C program
*******************************************************************************
*/

int counter = 0;
int result;

int main(void)
{
	int a = 4;
	int b = 5;
	static int sum = 10;

	result = 0;

	while (1)
	{
		if (++counter > 10)
		{
			result += a + b;
			a += 2;
			b += 3;
			counter = 0;
		}
	}

	return 0;
}
