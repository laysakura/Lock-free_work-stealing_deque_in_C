#define TEST_USE_TASK_ID

#define _GNU_SOURCE
#include "gsoc_taskqueue.h"
#include "gsoc_time.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <utmpx.h>
#include <unistd.h>


#ifndef TESTVAL_EXTENDS  /* It may be defined by Makefile */
#define TESTVAL_EXTENDS 1
#endif

typedef struct _worker {
  int id;
  cpu_set_t cpu;
  gsoc_taskqueue* my_taskq;
  gsoc_task* tasks;
  struct _worker* workers;
  long num_workers;
  int logged_worker;
} worker;


void* parallel_push_pop_take(void* s)
{
  worker* data = (worker*)s;
  gsoc_task* task;
  size_t victim;
  int i;
  pthread_setaffinity_np(pthread_self(), sizeof(data->cpu), &data->cpu);

  /* Think about fib where two tasks are created by one task */
  for (i = 0; i < GSOC_TASKQUEUE_INIT_SIZE * TESTVAL_EXTENDS / 2; ++i) {
    gsoc_taskqueue_push(data->my_taskq, &data->tasks[2*i]);
    gsoc_taskqueue_push(data->my_taskq, &data->tasks[2*i + 1]);
    task = gsoc_taskqueue_pop(data->my_taskq);
    if (task && data->id == data->logged_worker)
      printf("%lld is popped by CPU%d\n", task->test_id, sched_getcpu()); /* These values are evaluated by `make test' script */
  }

  /* All tasks are created, now just consume then (with other worker queue).
     If no task is in the queue, steal from others. */
  while (1) {
    task = gsoc_taskqueue_pop(data->my_taskq);
    if (task && data->id == data->logged_worker)
      printf("%lld is popped by CPU%d\n", task->test_id, sched_getcpu()); /* These values are evaluated by `make test' script */
    if (!task)
      {
        do
          {
            victim = random() % data->num_workers;
          }
        while (victim == data->id);
        task = gsoc_taskqueue_take(data->workers[victim].my_taskq);
        if (task && victim == data->logged_worker)
          printf("%lld is taken by CPU%d from CPU%d\n", task->test_id, sched_getcpu(), (int)victim); /* These values are evaluated by `make test' script */
        else
          return NULL;

      }
  }
}

int main()
{
  int i;

  /* for sequential use */
  gsoc_taskqueue* q;
  gsoc_task *tasks;

  /* for parallel use */
  long num_cpu = sysconf(_SC_NPROCESSORS_CONF);
  gsoc_taskqueue *taskqs[num_cpu];
  cpu_set_t cpuset;
  pthread_t tids[num_cpu];
  worker workers[num_cpu];
  int logged_worker = random() % num_cpu;
  double t1, t2;

  /* initializations for test */
  tasks = malloc(sizeof(gsoc_task) * GSOC_TASKQUEUE_INIT_SIZE * 100);
  for (i = 0; i < GSOC_TASKQUEUE_INIT_SIZE * 100; ++i)
    tasks[i].test_id = i;
  q = gsoc_taskqueue_new();
  for (i = 0; i < num_cpu; ++i)
    taskqs[i] = gsoc_taskqueue_new();

  /* push+pop */
  for (i = 0; i < GSOC_TASKQUEUE_INIT_SIZE * 100; ++i)
    gsoc_taskqueue_push(q, &tasks[i]);
  for (i = GSOC_TASKQUEUE_INIT_SIZE * 100 - 1; i >= 0 ; --i)
    assert(gsoc_taskqueue_pop(q)->test_id == i);
  /* pop for empty deque */
  assert(gsoc_taskqueue_pop(q) == NULL);

  /* push+take */
  for (i = 0; i < GSOC_TASKQUEUE_INIT_SIZE * 100; ++i)
    gsoc_taskqueue_push(q, &tasks[i]);
  for (i = 0; i < GSOC_TASKQUEUE_INIT_SIZE * 100; ++i)
    assert(gsoc_taskqueue_take(q)->test_id == i);
  /* take for empty deque */
  assert(gsoc_taskqueue_take(q) == NULL);


  /* Emulate workers */
  fprintf(stderr, "==========\nWith %d CPUs\n===========\n", (int)num_cpu);

  t1 = gettimeofday_sec();

  for (i = 0; i < num_cpu; ++i) {
    CPU_ZERO(&cpuset);
    CPU_SET(i, &cpuset);
    workers[i].cpu = cpuset;
    workers[i].id = i;
    workers[i].my_taskq = taskqs[i];
    workers[i].tasks = tasks;
    workers[i].workers = workers;
    workers[i].num_workers = num_cpu;
    workers[i].logged_worker = logged_worker;

    pthread_create(&tids[i], NULL, parallel_push_pop_take, &workers[i]);
  }
  for (i = 0; i < num_cpu; ++i)
    pthread_join(tids[i], NULL);

  t2 = gettimeofday_sec();

  fprintf(stderr, "%f sec elaplsed for parallel_push_pop_take()\n", t2-t1);

  /* finalization for test */
  for (i = 0; i < num_cpu; ++i)
    gsoc_taskqueue_delete(taskqs[i]);
  gsoc_taskqueue_delete(q);
  free(tasks);

  return 0;
}
