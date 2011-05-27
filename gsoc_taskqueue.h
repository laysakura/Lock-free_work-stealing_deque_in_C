/* Implemented after Chapter 16 of "The Art of Multiprocessor Programming"
   This is lock-free work stealing deque. */

#ifndef _GSOC_TASKQUEUE_H_
#define _GSOC_TASKQUEUE_H_


#include "gsoc_task_circular_array.h"

/* Constants */
#define GSOC_TASKQUEUE_INIT_SIZE 131072 /* just same as MassiveThreads */


typedef struct _gsoc_taskqueue {
  gsoc_task_circular_array* _taskqueue;       /* array of gsoc_task* */
  volatile size_t _top;         /* task is taken from _top of _taskqueue.
                                   _top is only incremented atomically and never decremented.
                                   It is checked by CAS when a task is poped/taken while
                                   there exist few tasks in _taskqueue. */
  volatile size_t _bottom;      /* task is pushed in _bottom of _taskqueue. */
} gsoc_taskqueue;

/* Methods for gsoc_taskqueue */
gsoc_taskqueue* gsoc_taskqueue_new();           /* constructor, initializer */
void gsoc_taskqueue_delete(gsoc_taskqueue* this); /* destructor */

void gsoc_taskqueue_push(gsoc_taskqueue* this, gsoc_task* task);
gsoc_task* gsoc_taskqueue_pop(gsoc_taskqueue* this);
gsoc_task* gsoc_taskqueue_take(gsoc_taskqueue* this);

#endif /* _GSOC_TASKQUEUE_H_ */
