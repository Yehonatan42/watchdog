#include "watchdog.h"

#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    int i = 0;
    int return_status = 0;

    return_status = StartWD(argc, argv, 1, 3);

    if (0 != return_status)
    {
        printf("Return status is %d\n", return_status);
    }
    
    while (i < 4)
    {  
        sleep(1);
        ++i;
    }
    
    StopWD();

    printf("Finished successfully!\n");

    return 0;
}