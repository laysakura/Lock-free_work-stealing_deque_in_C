#include "gsoc_taskqueue.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

gsoc_taskqueue* gsoc_taskqueue_new()
{
  gsoc_taskqueue* this;

  this = malloc(sizeof(gsoc_taskqueue));
  assert(this);

  this->_taskqueue = gsoc_task_circular_array_new(GSOC_TASKQUEUE_INIT_SIZE);
  this->_top = 0;
  this->_bottom = 1;

  return this;
}
void gsoc_taskqueue_delete(gsoc_taskqueue* this)
{
  gsoc_task_circular_array_delete(this->_taskqueue);
  free(this);
}

void gsoc_taskqueue_push(gsoc_taskqueue* this, gsoc_task* task)
{
  size_t old_top = this->_top;
  size_t num_tasks = this->_bottom - old_top;
  gsoc_task_circular_array* old_taskqueue = this->_taskqueue;

  if (__builtin_expect(num_tasks >= gsoc_task_circular_array_size(this->_taskqueue) - 1, 0))
    {
      /* _taskqueue needs to be expanded */
      this->_taskqueue = gsoc_task_circular_array_get_double_sized_copy(old_taskqueue);
      gsoc_task_circular_array_delete(old_taskqueue);
    }
  gsoc_task_circular_array_set(this->_taskqueue, this->_bottom, task);
  ++this->_bottom;
  __sync_synchronize(); /* new _bottom must be visible from take() called by another worker */
}

gsoc_task* gsoc_taskqueue_pop(gsoc_taskqueue* this)
{
  size_t old_top, new_top;
  size_t num_tasks;

  --this->_bottom;
  __sync_synchronize(); /* New _bottom must be visible from take() called by another worker.
                           Also, _top can be incremented by another worker. */
  old_top = this->_top;
  new_top = old_top + 1;
  num_tasks = this->_bottom - old_top; /* Note that num_tasks is less than real number of tasks by 1
                                          since _bottom is decremented. */
  if (__builtin_expect(num_tasks < 0, 0))
    {
      /* There is no task to pop. */
      this->_bottom = old_top;
      return NULL;
    }
  else if (__builtin_expect(num_tasks == 0, 0))
    {
      /* Both pop() and take() might be trying to get an only task in _taskqueue. */

      gsoc_task* ret = gsoc_task_circular_array_get(this->_taskqueue, this->_bottom);

      __sync_synchronize();  /* _top can be incremented by another worker. */
      if (!__sync_bool_compare_and_swap(&this->_top, old_top, new_top))
        /* take() already took the task */
        return NULL;
      else
        {
          this->_bottom = new_top;  /* Tell take() _taskqueue is empty */
          __sync_synchronize(); /* _bottom must be visible from take() */
          return ret;
        }
    }
  else
    /* There are some number of tasks safely popped */
    return gsoc_task_circular_array_get(this->_taskqueue, this->_bottom);
}

gsoc_task* gsoc_taskqueue_take(gsoc_taskqueue* this)
{
  size_t old_top, new_top;
  size_t old_bottom;
  size_t num_tasks;

  __sync_synchronize();  /* _top and _bottom can be changed by pop/push */
  old_top = this->_top;
  old_bottom = this->_bottom;
  new_top = old_top + 1;
  num_tasks = old_bottom - old_top;

  if (__builtin_expect(num_tasks <= 0, 0))
    return NULL;

  __sync_synchronize();  /* _top can be incremented by pop. */
  if (!__sync_bool_compare_and_swap(&this->_top, old_top, new_top))
    /* pop() already took the task */
    return NULL;
  else
    return gsoc_task_circular_array_get(this->_taskqueue, old_top);
}
