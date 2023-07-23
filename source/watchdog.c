#define _POSIX_SOURCE /*sigaction*/
#define _POSIX_C_SOURCE 200112L /*setenv*/

#include "watchdog.h"
#include "scheduler_heap.h"

#include <semaphore.h> /*sem_open, sem_t, sem_wait, sem_open*/
#include <pthread.h> /*pthread_create*/
#include <signal.h> /*sigaction, kill*/
#include <assert.h> /*assert*/
#include <stdlib.h> /*realloc, setenv*/
#include <sys/stat.h> /*S_IRWXU*/
#include <fcntl.h> /*O_CREAT*/
#include <errno.h> /* perror */
#include <unistd.h> /*pid_t*/
#include <stdio.h> /*sprintf*/
#include <string.h> /*atoi*/

#define UNUSED(x) (void)(x)

typedef struct thread_wrapper
{
    sem_t *user_process_sem;
    sem_t *com_thread_sem;
    char *file_to_exec;
    char **argv;
    scheduler_t *scheduler;
    int threshold;
} thread_wrapper_t; 

/*********************function declarations*****************************/
void Siguser1SignalHandler();
void Siguser2SignalHandler();
static void *WDThread(void *argv);
static int InitScheduler(int interval, size_t threshold, 
                                         thread_wrapper_t *thread_wrapper);
static int CleanUp(thread_wrapper_t *thread_wrapper);
static char **UserThreadInitialization(int argc, char **argv, int interval,
                                                              int threshold);
int SendSignalTask(void *param);
int CheckStatusTask(void *param);                                          

/*********************global variables*****************************/
pid_t g_opposite_pid = 0;
volatile sig_atomic_t g_counter = 0;
volatile sig_atomic_t g_stop = 0;
int g_i_am_wd = 0;
int g_need_to_recucitate = 0;
pthread_t com_thread = {0};

/***************************enums**********************************/
enum
{
    SUCCESS,
    FAILURE,
    PTHREAD_ERROR,
    SCHEDULER_ERROR,
    SETENV_FAILURE,
    SEMAPHORE_FAILURE,
    FORK_FAILURE,
    MALLOC_FAILURE
};

/*************************main functions*********************************/
int StartWD(int argc, char *argv[], int interval, int threshold)
{   
    thread_wrapper_t *thread_wrapper = {0};
    sem_t *user_process_sem = NULL;
    sem_t *com_thread_sem = NULL;
    char **args_to_pass = NULL;
    char *file_to_exec;
    int wd_pid = 0;
    int is_first_run = 0;
    int thread_return_status = 0;

    struct sigaction sigusr1_signal_handler = {0};
    struct sigaction sigusr2_signal_handler = {0};
    sigusr1_signal_handler.sa_handler = &Siguser1SignalHandler;  
    sigaction(SIGUSR1, &sigusr1_signal_handler, NULL);
    sigusr2_signal_handler.sa_handler = &Siguser2SignalHandler;  
    sigaction(SIGUSR2, &sigusr2_signal_handler, NULL);
    
    assert(0 < interval);
    assert(0 < threshold);
    assert(argv);
    
    thread_wrapper = malloc (sizeof(thread_wrapper_t)); 
    if (NULL == thread_wrapper)
    {
        return MALLOC_FAILURE;
    }
     
    user_process_sem = sem_open("wd_user_semaphore", O_CREAT, S_IRWXU, 0);
    com_thread_sem = sem_open("wd_com_semaphore", O_CREAT, S_IRWXU, 0);

    /*check if watchdog pid environment variable exists in order to determine*
     *if this is the first run and if not, get watchdog pid*/
    if (NULL == getenv("WD_PID"))
    {
        if (0 != setenv("WD_PID", "0", 1))
        {
            perror("setenv");
            return 1;
        }
        is_first_run = 1;
    }
    else
    {
        wd_pid = atoi(getenv("WD_PID"));
    }
    
    /*initialize arguments and program name to execute and determine if it's* 
     *watchdog thread or user thread*/
    if (getpid() != wd_pid)
    {
        file_to_exec = "./watchdog.out";
        args_to_pass = UserThreadInitialization(argc, argv, interval,
                                                            threshold);
        if (NULL == args_to_pass)
        {
            return FAILURE;
        }
    }    
    else
    {
        g_i_am_wd = 1;
        file_to_exec = argv[0];
    }
    
    g_opposite_pid = getppid();

    /*set wrapper that passes data into the scheduler tasks and WDThread* 
     *operations*/
    thread_wrapper->user_process_sem = user_process_sem;
    thread_wrapper->com_thread_sem = com_thread_sem;
    thread_wrapper->file_to_exec = file_to_exec;
    thread_wrapper->argv = args_to_pass;
    thread_wrapper->threshold = threshold;
    
    if (0 != InitScheduler(interval, threshold, thread_wrapper))
    {
        return SCHEDULER_ERROR;
    }

    /*if it's the first run, fork and execute the watchdog process, */
    if (is_first_run)
    {
        g_opposite_pid = fork();
    
        if (-1 == g_opposite_pid)
        {
            perror("fork");
            return FORK_FAILURE;
        }
        else if (0 == g_opposite_pid)
        {
            execvp(file_to_exec, args_to_pass);
        }  
         
        if (0 != kill(getpid(), SIGSTOP))
        {
            perror("kill");
            return FAILURE;
        }  
    }

    /*send communication threads to their function*/
    if (0 != pthread_create(&com_thread, NULL, WDThread, thread_wrapper))
    {
        return PTHREAD_ERROR;
    }

    /*if watchdog main thread, wait until its communication thread is finished*/
    if (g_i_am_wd)
    {
        thread_return_status = pthread_join(com_thread, NULL);
        if (0 != thread_return_status)
        {
            return thread_return_status;
        }
    }
    /*if user main thread, wait until sem_post is received from its 
     *communication thread*/
    else
    {
        if (0 != sem_wait(user_process_sem))
        {
            perror("sem_wait");
            return FAILURE;
        }
    }
    
    return SUCCESS;
}

int StopWD(void)
{
    int thread_return_status = 0;
    g_stop = 1;
    
    /*send signal to stop the watchdogs' scheduler*/
    if (0 != kill(g_opposite_pid, SIGUSR2))
    {
        perror("kill");
        return FAILURE;
    }
    
    /*wait for communication thread to finish cleaning up*/
    thread_return_status = pthread_join(com_thread, NULL);
    if (0 != thread_return_status)
    {
        return thread_return_status;
    }

    /*unset watchd pid environment variable and stop flag in order to reset in 
     *case of another run*/
    if (0 != unsetenv(("WD_PID")))
    {
        perror("unsetenv");
        return FAILURE;
    }
    g_stop = 0;

    return SUCCESS;
}

/*************************static functions*********************************/
static void *WDThread(void *argv)
{
    assert(argv);

    /*if watchdog thread, send signal to make user process continue, and if
     *user thread, send sem_post to release user main thread from semaphore*/
    if (g_i_am_wd)
    {
        kill(getppid(), SIGCONT);
    }
    else
    {
        if (0 != sem_post(((thread_wrapper_t *)argv)->user_process_sem))
        {
            perror("sem_post");
            return (void *)SEMAPHORE_FAILURE;
        }
    }

    /*in case a process has died and been revived, the revived thread sends
     *sem_post to release reviving thread from its semaphore*/
    if (0 != sem_post(((thread_wrapper_t *)argv)->com_thread_sem))
    {
        perror("sem_post");
        return (void *)SEMAPHORE_FAILURE;
    }

    /*main running and reviving loop*/
    while (!g_stop)
    {                         
        SchedulerRun(((thread_wrapper_t *)argv)->scheduler);
        
        if (1 == g_need_to_recucitate)
        {
            g_need_to_recucitate = 0;
            g_opposite_pid = fork();

            if (-1 == g_opposite_pid)
            {
                perror("fork");
                return (void *)FORK_FAILURE;
            }
            else if (0 == g_opposite_pid)
            {
                execvp(((thread_wrapper_t *)argv)->file_to_exec, 
                       ((thread_wrapper_t *)argv)->argv);
            }

            /*if user process, it stops itself to prevent further operations
             *until the watchdog has been revived and is ready to function*/
            if (!g_i_am_wd)
            {
                kill(getpid(), SIGSTOP);
            }

            /*wait until other thread is ready and sends sem_post*/
            if (sem_wait(((thread_wrapper_t *)argv)->com_thread_sem))
            {
                perror("sem_wait");
                return (void *)SEMAPHORE_FAILURE;  
            }
        }
    }

    CleanUp(argv);

    return (void *)SUCCESS;
}

static int InitScheduler(int interval, size_t threshold, 
                                       thread_wrapper_t *thread_wrapper)
{
    time_t curr_time = time(NULL);

    assert(interval > 0);
    assert(threshold > 0);
    assert(thread_wrapper);

    thread_wrapper->scheduler = SchedulerCreate();

    if (NULL == thread_wrapper->scheduler)
    {
        return SCHEDULER_ERROR;
    }

    /*add task that sends SIGUSR1 to the other process and increment counter*/
    SchedulerAddTask(thread_wrapper->scheduler, SendSignalTask, NULL, 
                                                                curr_time + 1, 
                                                                interval);

    /*add task that checks if counter is larger than threshold and if so,
     *revive the other process. also, check if stop flag has changed and if so
     *stop the scheduler without reviving the other process*/
    SchedulerAddTask(thread_wrapper->scheduler, CheckStatusTask,  
                                                thread_wrapper, 
                                                curr_time + 3, 
                                                interval * threshold);

    return SUCCESS;                                              
}

static int CleanUp(thread_wrapper_t *thread_wrapper)
{
    int i = 0;

    assert(thread_wrapper);

    SchedulerDestroy((thread_wrapper)->scheduler);
    
    while (NULL != thread_wrapper->argv[i])
    {
        free(thread_wrapper->argv[i]);
        ++i;
    }
    
    free(thread_wrapper->argv);
    
    if (0 != sem_close(thread_wrapper->com_thread_sem))
    {
        perror("sem_close");
        return SEMAPHORE_FAILURE;
    }
    if (0 != sem_unlink("wd_com_semaphore"))
    {
        perror("sem_unlink");
        return SEMAPHORE_FAILURE;
    }

    if (0 != sem_close(thread_wrapper->user_process_sem))
    {
        perror("sem_close");
        return SEMAPHORE_FAILURE;
    }
    if (0 != sem_unlink("wd_user_semaphore"))
    {
        perror("sem_unlink");
        return SEMAPHORE_FAILURE;
    }
    
    free(thread_wrapper);

    return SUCCESS;
}

static char **UserThreadInitialization(int argc, char **argv, int interval,
                                                              int threshold)
{
    char argv_buffer[11] = {0};
    int i = 0;
    char **args_to_pass = NULL;
    
    assert(argc > 0);
    assert(argv);
    assert(interval > 0);
    assert(threshold > 0);

    /*copy and organize arguments to later be passed when executing watchdog*/
    argc += 3;
    args_to_pass = malloc (sizeof(char *) * argc);
    
    if (NULL == args_to_pass)
    {
        return NULL;
    }
    
    args_to_pass[argc-1] = NULL;

    for (i = 0; i < argc - 3; ++i)
    {
        args_to_pass[i] = malloc (strlen(argv[i]) + 1);

        if (NULL == strcpy(args_to_pass[i], argv[i]))
        {
            return NULL;
        }
    }

    if (0 > sprintf(argv_buffer, "%d", interval))
    {
        return NULL;
    }
    args_to_pass[argc-3] = malloc (strlen(argv_buffer) + 1);
    if (NULL == strcpy(args_to_pass[argc-3], argv_buffer))
    {
        return NULL;
    }

    if (0 > sprintf(argv_buffer, "%d", threshold))
    {
        return NULL;
    }
    args_to_pass[argc-2] = malloc (strlen(argv_buffer) + 1);
    if (NULL == strcpy(args_to_pass[argc-2], argv_buffer))
    {
        return NULL;
    }

    return args_to_pass;
}

/*****************************signal handlers********************************/
void Siguser1SignalHandler()
{
    /*reset counter when receive signal from other process*/
    g_counter = 0;
}

void Siguser2SignalHandler()
{
    /*set flag to release thread from its reviving loop when StopWD is called*/
    g_stop = 1;
}

/*********************************tasks**************************************/
int SendSignalTask(void *param)
{
    /*task to send signal to ther process and increment own counter*/
    ++g_counter; 
    kill(g_opposite_pid, SIGUSR1);

    return SUCCESS; 

    UNUSED(param);
}

int CheckStatusTask(void *thread_wrapper)
{
    assert(thread_wrapper);

    /*if stop flag has changed, send SIGUSR to user process and both processes
     *stop their schedulers*/
    if (g_stop)
    {
        if (g_i_am_wd)
        {
            kill(g_opposite_pid, SIGUSR2);
        }
        
        SchedulerStop(((thread_wrapper_t *)thread_wrapper)->scheduler);
        
        return SUCCESS;
    }

    /*if counter has reached the threshold, revive the other process*/
    if (g_counter >= ((thread_wrapper_t *)thread_wrapper)->threshold)
    {
        g_need_to_recucitate = 1;
        SchedulerStop(((thread_wrapper_t *)thread_wrapper)->scheduler);
    }

    return SUCCESS;
}

