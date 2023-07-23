#define _POSIX_C_SOURCE 200112L /*setenv*/

#include <stdlib.h> /*atoi, setenv*/
#include <string.h> /*sprintf*/
#include <stdio.h> /*perror*/
#include <sys/types.h> /*getpid*/
#include <unistd.h> /*getpid*/
#include <signal.h> /*kill*/
#include <assert.h> /*assert*/
#include <string.h> /*stcpy*/

#include "watchdog.h"

int main(int argc, char *argv[])
{
    int interval = 0;
    size_t threshold = 0;
    char buffer[10] = {0};
    
    assert(argv);

    /*set up interval and threshold received via arguments*/
    interval = atoi(argv[argc-2]);
    threshold = atoi(argv[argc-1]);
    argv[argc-2] = NULL;

    /*set environment variable that holds the watchdog pid*/
    if (0 > sprintf(buffer, "%d", getpid()))
    {
        return 1;
    }
    if (0 != setenv("WD_PID", buffer, 1))
    {
        perror("setenv");
        return 1;
    }
    
    if (0 != StartWD(argc, argv, interval, threshold))
    {
        return 1;
    }

    return 0;
}
