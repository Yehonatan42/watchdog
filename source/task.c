#include <assert.h> /*assert*/
#include <stdlib.h> /*malloc, free*/

#include "task.h"

struct task
{
	uid_ty uid;
	op_func_t op_func;
	void *param;
};

task_t  *TaskCreate(void *param, op_func_t op_func)
{
	task_t *new_task = NULL;
	
	assert(op_func);
	
	new_task = malloc (sizeof(task_t));
	
	if (NULL == new_task)
	{
		return NULL;
	}
	new_task->uid = UIDCreate();
	new_task->op_func = op_func;
	new_task->param = param;
		
	if (UIDIsEqual(new_task->uid, bad_uid))
	{
		free(new_task);	new_task = NULL;
	}
	
	return (new_task);	
}

void TaskDestroy(task_t *task)
{
	assert(task);
	
	free(task); task = NULL;
}

int TaskExecute(task_t *task)
{
	assert(task);
	assert(task->op_func);
	
	return (task->op_func(task->param));
}

void TaskSetParam(task_t *task, void *param)
{
	assert(task);
	
	task->param = param;
}

void *TaskGetParam(task_t *task)
{
	assert(task);
	
	return(task->param);		
}

uid_ty TaskGetUid(task_t *task)
{
	assert(task);
	
	return (task->uid);
}


