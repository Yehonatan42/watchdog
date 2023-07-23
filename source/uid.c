#include <arpa/inet.h> /* getifaddrs() */
#include <ifaddrs.h> /* getifaddrs() */
#include <string.h> /*strcpy, strcmp*/

#include "uid.h"

static size_t counter = 0;

const uid_ty bad_uid = {0, -1, 0, "a1b2c3"};
	
static char *GetIp()
{
	struct ifaddrs *ifap, *ifa;
	struct sockaddr_in *sa;
	char *addr = NULL;

	if (-1 != getifaddrs(&ifap))
	{
		for (ifa = ifap; ifa; ifa = ifa->ifa_next) 
		{
			if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) 
			{
				sa = (struct sockaddr_in *) ifa->ifa_addr;
				addr = inet_ntoa(sa->sin_addr);
				break;
			}
		}		
	}

	freeifaddrs(ifap);
	
	return (addr);
}

uid_ty UIDCreate(void)
{
	uid_ty new_uid;
	new_uid.pid = getpid();
	new_uid.counter = ++counter;
	new_uid.time = time(0);
	
	if (-1 == new_uid.time)
	{
		return (bad_uid);	
	}
	if (NULL == memcpy(new_uid.ip_address ,GetIp(), 14))
	{
		return (bad_uid);
	}
	
    return (new_uid);
}

int UIDIsEqual(uid_ty uid_1, uid_ty uid_2)
{
	int status = 0;
	status += (uid_1.pid == uid_2.pid);
	status += (uid_1.time == uid_2.time);
	status += (uid_1.counter == uid_2.counter);
	status += (0 == strcmp(uid_1.ip_address, uid_2. ip_address));
	
	return (4 == status);															
}

