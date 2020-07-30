//Yair Cohen 318571718 LATE-SUBMISSION
#ifndef __THREAD_POOL__
#define __THREAD_POOL__
#include "pthread.h"
#include "osqueue.h"
#include "stdlib.h"
#include <stdio.h>
#include <fcntl.h>
/**
 * a struct that holds all the data related to the threadpool.
 */
typedef struct thread_pool
{
    int threads_lim;
    pthread_t* threadsArr;
    OSQueue* myQueue;
    pthread_mutex_t pthreadMutex;
    pthread_cond_t condition;
    int stat;
}ThreadPool;
/**
 * This func creates the threadpool initializes the pool.
 * @param numOfThreads -  the limit number of threads allowed to run.
 * @return a pointer to the threadpool.
 */
ThreadPool* tpCreate(int numOfThreads);
/**
 * This func destroys the thread pool  and is responsible to free the allocated memory.
 * @param threadPool a pointer to the threadpool.
 * @param shouldWaitForTasks a flag whether there are tasks still running.
 */
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);
/**
 *  This func inserts a task to the queue, its pointer to the task function and the arguments.
 * @param threadPool a pointer to the threadpool.
 * @param computeFunc - the task function by pointer.
 * @param param the parametes to the task function.
 * @return whether it succeeded.
 */
int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
