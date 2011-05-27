#ifndef _GSOC_TASK_H_
#define _GSOC_TASK_H_


typedef struct gsoc_task {
  void* result;
  

  struct gsoc_task* parent;
  struct gsoc_task** children;

#ifdef TEST_USE_TASK_ID
  unsigned long long test_id;
#endif
} gsoc_task;


#endif /* _GSOC_TASK_H_ */
