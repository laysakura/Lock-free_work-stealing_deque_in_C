#define TEST_USE_TASK_ID
#define _GNU_SOURCE
#include "gsoc_task_circular_array.h"
#include "gsoc_time.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/* Used in test_change_size_and_checkit() */
#define INITIAL_SIZE (1 << 1)
#define END_SIZE (1 << 20)

/* Used in test_max_capacity() */
#define MAX_SIZE (1 << 29)

#define NUM_ROUND 5

void test_set_get(gsoc_task_circular_array* this, gsoc_task* test_tasks)
{
  unsigned long long i;
  for (i = 0;
       i < gsoc_task_circular_array_size(this) * NUM_ROUND
         && i < (1 << 31);
       ++i)
    {
      test_tasks[i].test_id = i;
      gsoc_task_circular_array_set(this, i, &test_tasks[i]);
    }
  for (i = 0;
       i < gsoc_task_circular_array_size(this) * NUM_ROUND
         && i < (1 << 31);
       ++i)
    {
      assert((i % gsoc_task_circular_array_size(this)) + gsoc_task_circular_array_size(this) * (NUM_ROUND - 1)
             == gsoc_task_circular_array_get(this, i)->test_id);
    }
}

void test_change_size_and_checkit(gsoc_task_circular_array* would_be_freed, gsoc_task* test_tasks)
{
  gsoc_task_circular_array* arr = would_be_freed;
  gsoc_task_circular_array* new_arr;
  unsigned long long size = gsoc_task_circular_array_size(would_be_freed), size_after_expansion;
  size_after_expansion = 2 * size;
  while (size_after_expansion < END_SIZE)
    {
      new_arr = gsoc_task_circular_array_get_double_sized_copy(arr);
      gsoc_task_circular_array_delete(arr);
      arr = new_arr;

      assert(size_after_expansion == gsoc_task_circular_array_size(arr));
      test_set_get(arr, test_tasks);

      size_after_expansion *= 2;
    }
  gsoc_task_circular_array_delete(arr);
}

void test_max_capacity(gsoc_task_circular_array* this, gsoc_task* test_tasks)
{
  unsigned long long i;
  for (i = 0; i < MAX_SIZE; ++i)
    {
      test_tasks[i].test_id = i;
      gsoc_task_circular_array_set(this, i, &test_tasks[i]);
      assert(i == gsoc_task_circular_array_get(this, i)->test_id);
    }
}

int main()
{
  gsoc_task_circular_array *carray;
  gsoc_task* test_tasks;
  double t1, t2;

  t1 = gettimeofday_sec();

  carray = gsoc_task_circular_array_new(INITIAL_SIZE);
  test_tasks = malloc(sizeof(gsoc_task) * END_SIZE * NUM_ROUND);
  assert(test_tasks);
  test_change_size_and_checkit(carray, test_tasks); /* carray is freed in callee */

  carray = gsoc_task_circular_array_new(INITIAL_SIZE);
  test_tasks = malloc(sizeof(gsoc_task) * MAX_SIZE);
  assert(test_tasks);
  test_max_capacity(carray, test_tasks);

  t2 = gettimeofday_sec();

  fprintf(stderr, "%f elapsed\n", t2 - t1);

  free(test_tasks);
  free(carray);

  return 0;
}
