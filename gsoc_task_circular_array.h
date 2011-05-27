/* Circular array implemented after Chapter 16 of "The Art of Multiprocessor Programming"
   Always use from push/pop/take of taskqueue. task_circular_array itself is not thread safe. */

#ifndef _GSOC_TASK_CIRCULAR_ARRAY_H_
#define _GSOC_TASK_CIRCULAR_ARRAY_H_

#include "gsoc_task.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct _gsoc_task_circular_array {
  gsoc_task** _array;
  unsigned long long _size;
} gsoc_task_circular_array;

static inline gsoc_task_circular_array*
gsoc_task_circular_array_new(unsigned long long capacity)
{
  gsoc_task_circular_array* this;

  this = malloc(sizeof(gsoc_task_circular_array));
  assert(this);

  this->_array = malloc(sizeof(gsoc_task*) * capacity);
  assert(this->_array);

  this->_size = capacity;

  return this;
}


static inline void
gsoc_task_circular_array_delete(gsoc_task_circular_array* this)
{
  free(this->_array);
  free(this);
}


static inline gsoc_task*
gsoc_task_circular_array_get(gsoc_task_circular_array* this, unsigned long long index)
{
  return this->_array[index % this->_size];
}


static inline void
gsoc_task_circular_array_set(gsoc_task_circular_array* this, unsigned long long index, gsoc_task* task)
{
  this->_array[index % this->_size] = task;
}


static inline gsoc_task_circular_array*
gsoc_task_circular_array_get_double_sized_copy(gsoc_task_circular_array* old)
{
  gsoc_task_circular_array* new_arr;
  if (old->_size * 2 >= (1 << 31))
    {
      fprintf(stderr, "gsoc_task_circular_array cannot deal with more than 2^31 tasks.\n");
      exit(1);
    }
  new_arr = gsoc_task_circular_array_new(old->_size * 2);
  memcpy(&new_arr->_array[0], &old->_array[0],
         sizeof(gsoc_task*) * old->_size);
  return new_arr;
}

static inline unsigned long long
gsoc_task_circular_array_size(gsoc_task_circular_array* this)
{
  return this->_size;
}


#endif /* _GSOC_TASK_CIRCULAR_ARRAY_H_ */
