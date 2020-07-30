//Yair Cohen 318571718 LATE-SUBMISSION
#include "threadPool.h"
/**
 *  a struct meant to help holding the data of a given task.
 */
typedef struct task
{
    void (*func)(void *args);
    void* args;
}Task;
/**
 * this func runs the task function with the given params.
 * @param args  the arguments of the function.
 * @return
 */
void* runTask(void* args) {
    Task* task;
    ThreadPool* threadPool = (ThreadPool*)args;
    struct os_queue* taskQueue = threadPool->myQueue;
    while ((threadPool->stat< 2) && (!osIsQueueEmpty(taskQueue)))
    {
        // locking the critical section and avoiding busy waiting.
        pthread_mutex_lock(&(threadPool->pthreadMutex));
        if(osIsQueueEmpty(taskQueue) && (threadPool->stat < 2))
        {
            //avoiding busy waiting by pthread conditions.
            pthread_cond_wait(&(threadPool->condition), &(threadPool->pthreadMutex));
        }
        // as long as the queue is not empty.
        if (!(osIsQueueEmpty(taskQueue)))
        {
            // taking a job from the queue.
            task = osDequeue(taskQueue);
            pthread_mutex_unlock(&(threadPool->pthreadMutex));
        }

        //checking if a task was pulled from the queue, if so execute it.
        if (task != NULL)
        {
            task->func(task->args);
            // once it over freeing the memory.
            free(task);
        }
    }
}
/**
 * This func creates the threadpool initializes the pool.
 * @param numOfThreads -  the limit number of threads allowed to run.
 * @return a pointer to the threadpool.
 */
ThreadPool* tpCreate(int numOfThreads)
{
    // creating the thread pool by allocating with malloc.
    ThreadPool* threadPool = (ThreadPool*)malloc(sizeof(ThreadPool));
    int i;
    // checking if allction was successful.
    if (threadPool == NULL)
    {
        printf("Couldn't allocate threadpool.\n");
        return NULL;
    }
    threadPool->threads_lim = numOfThreads;
    // creating the threads array with malloc. a pre defined size by arg.
    threadPool->threadsArr = (pthread_t*)malloc(sizeof(pthread_t) * threadPool->threads_lim);
    // checking if allction was successful.
    if (threadPool->threadsArr == NULL) {
        printf("Couldn't allocate threads array.\n");
        return NULL;
    }
    threadPool->myQueue = osCreateQueue();
    threadPool->stat = 0;
    // checking the mutex and pthread condition.
    if (pthread_mutex_init(&(threadPool->pthreadMutex), NULL) != 0 || pthread_cond_init(&(threadPool->condition), NULL) != 0)
    {
        tpDestroy(threadPool, 0);
        return NULL;
    }

    for (i = 0; i < threadPool->threads_lim; i++)
    {
        pthread_create(&(threadPool->threadsArr[i]), NULL, runTask, (void *)threadPool);
    }

    return threadPool;
}
/**
 * This func destroys the thread pool  and is responsible to free the allocated memory.
 * @param threadPool a pointer to the threadpool.
 * @param shouldWaitForTasks a flag whether there are tasks still running.
 */
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks)
{
    int i;
    if (threadPool == NULL)
        return;

    pthread_mutex_lock(&threadPool->pthreadMutex);
    if (threadPool->stat == 0)
        threadPool->stat = 1;
    else
        return;

    pthread_mutex_unlock(&threadPool->pthreadMutex);
    if (shouldWaitForTasks == 0)
        threadPool->stat = 2;
    pthread_mutex_lock(&(threadPool->pthreadMutex));
    if((pthread_cond_broadcast(&(threadPool->condition)) != 0) ||
       (pthread_mutex_unlock(&(threadPool->pthreadMutex)) != 0))
    {
        exit(1);
    }
    for (i = 0; i < threadPool->threads_lim; i++) {
        pthread_join(threadPool->threadsArr[i], NULL);
    }

    threadPool->stat = 2;
    while (!osIsQueueEmpty(threadPool->myQueue)) {
        Task* task = osDequeue(threadPool->myQueue);
        free(task);
    }
    osDestroyQueue(threadPool->myQueue);
    free(threadPool->threadsArr);
    pthread_mutex_destroy(&(threadPool->pthreadMutex));
    free(threadPool);
}
/**
 *  This func inserts a task to the queue, its pointer to the task function and the arguments.
 * @param threadPool a pointer to the threadpool.
 * @param computeFunc - the task function by pointer.
 * @param param the parametes to the task function.
 * @return whether it succeeded.
 */
int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param)
{
    if(threadPool == NULL || computeFunc == NULL)
    {
        return -1;
    }
    if (threadPool->stat==0) {
        Task *task = (Task*) malloc(sizeof(Task));
        if (task == NULL) {
            return -1;
        }
        task->args = param;
        task->func = computeFunc;
        pthread_mutex_lock(&(threadPool->pthreadMutex));
        osEnqueue(threadPool->myQueue, (void *) task);
        if (pthread_cond_signal(&(threadPool->condition)) != 0) {
            exit(1);
        }
        pthread_mutex_unlock(&(threadPool->pthreadMutex));
        return 0;
    } else
        return -1;
}