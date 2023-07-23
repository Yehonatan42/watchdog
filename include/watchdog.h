#include <stddef.h> /* size_t */

int StartWD(int argc, char *argv[], int interval, int threshold);
/* Description: Starts running the watchdog process in order to recucitate the
 * user process in case of crash.
 * Parameters: Int argc that is the number of arguments.
 * Char array that contains the user program name and arguments.
 * Path to watchdog binary.
 * Interval time between each signal that checks if the process is alive.
 * Grace counter to determine how many missed signals are allowed before
 * recucitation.
 * Return value: Int that describes a return status. */

int StopWD(void);
/* Description: Stops the watchdog process so that the user program can 
 * terminate. */


