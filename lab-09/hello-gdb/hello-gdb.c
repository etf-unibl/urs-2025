#include <stdio.h>
#include <string.h>

char* msg = "Hello GDB World! I received x.\n";

int main(int argc, char *argv[])
{
    if ((argc == 1) || (argc > 2))
    {
        printf("Please provide a single character as an argument!\n");
        
        return 1;
    }
    else
    {
        if (strlen(argv[1]) > 1)
        {
            printf("Please provide a single character as an argument!\n");
            
            return 1;
        }
        else
        {
            // Replace x with a character received via command line
            msg[28] = argv[1][0];
            printf("%s", msg);
        }
    }

    return 0;
}

