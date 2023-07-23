#include "uid.h" /* included in order to generate uid for task */
 
typedef int (*op_func_t)(void *param);
typedef struct task task_t;

task_t  *TaskCreate(void *param, op_func_t op_func);

/* Description: Creates a task in task_t format containing a uid, function 
 * pointer and void pointer paramater for the functions' argument.
 * Return value: task_t struct pointer to the task.
 * Time complexity: O(1)  */

void TaskDestroy(task_t *task);
/* Description: Destroys a given task. 
 * Return value: NON.
 * Time complexity: O(1) */

int TaskExecute(task_t *task);
/* Description: Calls the function specified in the task.
 * Return value: Return status of whatever function was called.
 * Time complexity: O(1) */

void TaskSetParam(task_t *task, void *param);
/* Description: Changes the parameter (void pointer) in the task.
 * Return value: NON.
 * Time complexity: O(1) */

void *TaskGetParam(task_t *task);
/* Description: Gets the parameter in the task.
 * Return value: Void pointer to the parameter.
 * Time complexity: O(1) */

uid_ty TaskGetUid(task_t *task);
/* Description: Gets the uid of a task.
 * Return value: uid of the task
 * Time complexity: O(1) */


