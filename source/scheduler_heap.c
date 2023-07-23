#include <assert.h> /*assert*/
#include <stdlib.h> /*free, malloc*/

#include "task.h" /* creating and manipulatin tasks */
#include "scheduler_heap.h"
#include "pq_heap.h" 

typedef struct scheduled_task
{
	time_t execution_time;
	size_t interval_time;
	task_t *task;
}scheduled_task_t;

struct scheduler
{
	pq_heap_t *scheduled_tasks;
	int stop_flag;
};

static int UIDIsMatch(const void *task_uid, const void *task)
{
	scheduled_task_t *searched_task = NULL;
	
	assert(task_uid);
	assert(task);
	
	searched_task = (scheduled_task_t*)task;
	
	return UIDIsEqual(*(uid_ty*)task_uid, TaskGetUid(searched_task->task));		
}

static int TimeCmp(const void *inserted_value, const void *current_position)
{
	scheduled_task_t *new_task = (scheduled_task_t *)inserted_value;
	scheduled_task_t *list_task = (scheduled_task_t *)current_position;
	 
	return (list_task->execution_time - new_task->execution_time);  
}

static void ScheduledTaskDestroy(scheduled_task_t *scheduled_task)
{
	assert(scheduled_task);
	assert(scheduled_task->task);
	
	TaskDestroy(scheduled_task->task); scheduled_task->task = NULL;
	free(scheduled_task); scheduled_task = NULL;
}

scheduler_t *SchedulerCreate(void)
{
	scheduler_t *new_scheduler = malloc (sizeof(scheduler_t));
	
	if(NULL == new_scheduler)
	{
		return NULL;	
	}
	new_scheduler->scheduled_tasks = PQHeapCreate(TimeCmp);
	
	return (new_scheduler);	
}

void SchedulerDestroy(scheduler_t *scheduler)
{
	assert(scheduler);
	assert(scheduler->scheduled_tasks);
	
	SchedulerClear(scheduler);
	PQHeapDestroy(scheduler->scheduled_tasks);
	free(scheduler);
	scheduler = NULL;
}										   

int SchedulerRemoveTask(scheduler_t *scheduler, uid_ty task_uid)
{
	int return_status = 1;
	void *task_uid_p = &task_uid;
	scheduled_task_t *task_to_remove = NULL;
	
	assert(scheduler);
	
	task_to_remove = (scheduled_task_t*)PQHeapErase(scheduler->scheduled_tasks,
						    UIDIsMatch, 
						    task_uid_p);
	if (NULL != task_to_remove)
	{
		ScheduledTaskDestroy(task_to_remove);
		return_status = 0;	
	}								
	
	return (return_status);
}

void SchedulerClear(scheduler_t *scheduler)
{
	scheduled_task_t *task_to_remove = NULL;
	
	assert(scheduler);
	assert(scheduler->scheduled_tasks);
	
	while (1 != PQHeapIsEmpty(scheduler->scheduled_tasks))
	{
		task_to_remove = PQHeapDequeue(scheduler->scheduled_tasks);
		ScheduledTaskDestroy(task_to_remove);				
	}
}
 
size_t SchedulerSize(const scheduler_t *scheduler)
{
	assert(scheduler);
	assert(scheduler->scheduled_tasks);
	
	return (PQHeapSize(scheduler->scheduled_tasks));
}

int SchedulerIsEmpty(const scheduler_t *scheduler)
{
	assert(scheduler);
	assert(scheduler->scheduled_tasks);
	
	return (PQHeapIsEmpty(scheduler->scheduled_tasks));
}

uid_ty SchedulerAddTask(scheduler_t *scheduler, action_func_t op_func, 
					        void *param, 
					        time_t execution_time, 
					        size_t interval_time)
{
	scheduled_task_t *new_scheduled_task = NULL;
	
	assert(scheduler);	
	assert(op_func);	
	
	new_scheduled_task = malloc (sizeof(scheduled_task_t));
	
	if (NULL == new_scheduled_task)
	{
		return bad_uid;	
	}
	new_scheduled_task->execution_time = execution_time;
	new_scheduled_task->task = TaskCreate(param, op_func);
	new_scheduled_task->interval_time = interval_time;
	
	PQHeapEnqueue(scheduler->scheduled_tasks, new_scheduled_task);
	
	return (TaskGetUid(new_scheduled_task->task));
}	

int SchedulerRun(scheduler_t *scheduler)
{
	int return_status = 0;
	scheduled_task_t *current_scheduled_task = NULL;
	scheduler->stop_flag = 0;
	
	assert(scheduler);
	
	while (1 != SchedulerIsEmpty(scheduler) && 1 != scheduler->stop_flag)
	{
		current_scheduled_task = PQHeapDequeue(scheduler->scheduled_tasks);
		
		while (current_scheduled_task->execution_time > time(NULL))
		{
			/*wait for scheduled time*/	
		}
		
		if (-1 == time(NULL))
		{
			PQHeapEnqueue(scheduler->scheduled_tasks, 
			 	  current_scheduled_task);
			return -1;
		}
		
		if (0 != TaskExecute(current_scheduled_task->task))	
		{
			return_status = -1;
			ScheduledTaskDestroy(current_scheduled_task);
		}

		else
		{ 
			if (0 != current_scheduled_task->interval_time)
			{
				current_scheduled_task->execution_time += 
				current_scheduled_task->interval_time;
				PQHeapEnqueue(scheduler->scheduled_tasks, 
				current_scheduled_task);	
			}
			else
			{
				ScheduledTaskDestroy(current_scheduled_task);
			}	
		}
		if (1 == scheduler->stop_flag)
		{
			return_status = 1;
		}
	}
	
	return return_status;

}

void SchedulerStop(scheduler_t *scheduler)
{
	scheduler->stop_flag = 1;
}


