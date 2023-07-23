#include <unistd.h> /* pid_t */
#include <sys/types.h> /* pid_t */
#include <time.h> /* time */
#include <stddef.h> /* size_t */

typedef struct uid
{
    pid_t pid;
    time_t time;
	size_t counter;
	char ip_address[14];
} uid_ty;

extern const uid_ty bad_uid;  /* You should assign the appropriate value */

uid_ty UIDCreate(void);

int UIDIsEqual(uid_ty uid_1, uid_ty uid_2);

 
