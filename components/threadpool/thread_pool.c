#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "thread_pool.h"

static void *thread_task_work(void *arg)
{
    PTR_CHECK_NULL(arg);

    THREAD_POOL_T *pool  = (THREAD_POOL_T *)arg;
    THREAD_EVENT_T event = {0};

    while (1)
    {
        pthread_mutex_lock(&(pool->lock));

        /* task exit */
        if (THREAD_POOL_STOP == pool->status)
        {
            pthread_mutex_unlock(&(pool->lock));
            break;
        }

        if (sdp_queue_empty(pool->event_queue))
        {
            pthread_mutex_unlock(&(pool->lock));
            continue;
        }

        /* wait event */
        //pthread_cond_wait(&(pool->notify), &(pool->lock));

        /* get event */
        if (0 > sdp_dequeue(pool->event_queue, (void *)(&event), sizeof(event)))
        {
            printf("failed to get event\n");
            pthread_mutex_unlock(&(pool->lock));
            continue;
        }

        pthread_mutex_unlock(&(pool->lock));

        /* handle event */
        event.func(event.arg, event.arg_size);
    }

    return NULL;
}

THREAD_POOL_T *thread_pool_create(int thread_num, int event_queue_num)
{
    int i = 0;
    THREAD_POOL_T *pool = NULL;

    /* create thread pool */
    pool = (THREAD_POOL_T *)malloc(sizeof(*pool));
    if (!pool)
    {
        printf("ERROR : Can not create thread pool\n");
        return NULL;
    }
    memset(pool, 0, sizeof(*pool));

    /* create thread tasks */
    pool->tasks = (THRAED_TASK_T *)malloc(sizeof(THRAED_TASK_T) * thread_num);
    if (!pool->tasks)
    {
        printf("ERROR : Can not create thread tasks\n");
        free(pool);
        return NULL;
    }
    memset(pool->tasks, 0, sizeof(THRAED_TASK_T) * thread_num);

    /* create event queue */
    pool->event_queue = sdp_queue_create(event_queue_num, sizeof(THREAD_EVENT_T));
    if (!pool->event_queue)
    {
        printf("ERROR : Can not create event queue\n");
        free(pool->tasks);
        free(pool);
        return NULL;
    }

    /* create thread lock */
    pthread_mutex_init(&(pool->lock), NULL);

    /* create thread cond */
    pthread_cond_init(&(pool->notify), NULL);

    /* updata thread pool status to running */
    pool->status = THREAD_POOL_RUNNING;

    /* create thread task */
    for (i = 0; i < thread_num; ++i)
    {
        if (0 > pthread_create(&(pool->tasks[i].pthread), NULL, thread_task_work, (void *)pool))
        {
            printf("ERROR : Can not create thread tasks\n");
            goto TASK_CREATE_FAILD;
        }

        pool->working_num++;
    }

    return pool;

TASK_CREATE_FAILD :

    /* free resource */
    thread_pool_destory(pool, 1);

    return NULL;
}

int thread_pool_destory(THREAD_POOL_T *pool, int force)
{
    PTR_CHECK_N1(pool)
    
    int i = 0;

    if (!force)
    {
        while (pool->working_num && !sdp_queue_empty(pool->event_queue))
        {
            usleep(200);
        }
    }

    /* updata thread pool status to stop */
    pool->status = THREAD_POOL_STOP;

    /* wait task */
    for (i = 0; i < pool->working_num; ++i)
    {
        pthread_join(pool->tasks[i].pthread, NULL);
    }

    /* free resource */
    free(pool->tasks);
    sdp_queue_free(pool->event_queue);
    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->notify));
    free(pool);

    return 0;
}

int thread_event_add(THREAD_POOL_T *pool, thread_event_func_t func, void *arg, int arg_size)
{
    PTR_CHECK_N1(pool);
    PTR_CHECK_N1(func);
    PTR_CHECK_N1(arg);

    /* new event */
    THREAD_EVENT_T event = {
        .func     = func,
        .arg      = arg,
        .arg_size = arg_size
    };

    pthread_mutex_lock(&(pool->lock));

    /* submit event */
    if (0 > sdp_enqueue(pool->event_queue, (void *)(&event), sizeof(event)))
    {
        pthread_mutex_unlock(&(pool->lock));
        return -1;
    }

    /* report event */
    // pthread_cond_signal(&(pool->notify));

    pthread_mutex_unlock(&(pool->lock));

    return 0;
}