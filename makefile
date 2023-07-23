CC =gcc 
CFLAGS =-fPIC -Iinclude -pthread -ansi -pedantic-errors -Wall -Wextra -g
LDFLAGS =-shared   
RM =rm -f  
TARGET_LIB =./libwatchdog.so  
DRIVER = ./watchdog.out
USER_PROGRAM = ./user_program.out
SRCS = ./source/heap.c ./source/pq_heap.c ./source/scheduler_heap.c ./source/task.c ./source/uid.c ./source/vector.c ./source/watchdog.c
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: ${TARGET_LIB} $(DRIVER) $(USER_PROGRAM)

$(DRIVER): source/watchdog_driver.c ${TARGET_LIB}
	$(CC) $(CFLAGS) ./source/watchdog_driver.c -I. -L$(CURDIR) -lwatchdog -o watchdog.out

$(USER_PROGRAM): test/user_program.c ${TARGET_LIB}
	$(CC) $(CFLAGS) ./test/user_program.c -I. -L$(CURDIR) -lwatchdog -o user_program.out

.PHONY: library
library: ${TARGET_LIB}

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean_all
clean_all: 
	rm ${TARGET_LIB} $(DRIVER) $(USER_PROGRAM) ./*/*.o ./*/*.d

.PHONY: clean
clean: 
	rm ./*/*.o ./*/*.d	
