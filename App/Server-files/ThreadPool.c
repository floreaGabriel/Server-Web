#include "ThreadPool.h"
#include <stdlib.h>
#include <stdio.h>

// Funcție rulată de fiecare thread
static void *threadWorker(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;

    while (1) {
        Task *task;

        // Blocare mutex pentru a accesa coada
        pthread_mutex_lock(&pool->queueMutex);

        // Așteaptă până când există task-uri sau pool-ul este oprit
        while (pool->taskQueueHead == NULL && !pool->stop) {
            pthread_cond_wait(&pool->condition, &pool->queueMutex);
        }

        if (pool->stop && pool->taskQueueHead == NULL) {
            pthread_mutex_unlock(&pool->queueMutex);
            break;
        }

        // Preluare task din coadă
        task = pool->taskQueueHead;
        pool->taskQueueHead = task->next;
        if (pool->taskQueueHead == NULL) {
            pool->taskQueueTail = NULL;
        }

        pthread_mutex_unlock(&pool->queueMutex);

        // Execută task-ul
        if (task) {
            task->function(task->arg);
            pool->queueSize--;
            free(task);
        }
    }

    return NULL;
}

// Creare ThreadPool
ThreadPool *threadPoolCreate(size_t numThreads) {
    ThreadPool *pool = (ThreadPool *)malloc(sizeof(ThreadPool));
    if (!pool) {
        perror("Eroare alocare memorie pentru ThreadPool");
        return NULL;
    }

    pool->threadCount = numThreads;
    pool->stop = 0;
    pool->taskQueueHead = NULL;
    pool->taskQueueTail = NULL;
    pool->queueSize=0;



    pthread_mutex_init(&pool->queueMutex, NULL);
    pthread_cond_init(&pool->condition, NULL);

    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * numThreads);
    if (!pool->threads) {
        perror("Eroare alocare memorie pentru thread-uri");
        free(pool);
        return NULL;
    }

    for (size_t i = 0; i < numThreads; i++) {
        if (pthread_create(&pool->threads[i], NULL, threadWorker, pool) != 0) {
            perror("Eroare creare thread");
            threadPoolDestroy(pool);
            return NULL;
        }
    }

    return pool;
}

// Adăugare task în coadă
void threadPoolEnqueue(ThreadPool *pool, void (*function)(void *), void *arg) {
    if (!pool || !function) return;


    pthread_mutex_lock(&pool->queueMutex);

    if (pool->queueSize >= pool->threadCount) {
        pthread_mutex_unlock(&pool->queueMutex);
        fprintf(stderr, "Coada ThreadPool este plină! Task-ul a fost respins.\n");
        return;
    }

    Task *newTask = (Task *)malloc(sizeof(Task));
    if (!newTask) {
        perror("Eroare alocare memorie pentru task");
        return;
    }

    newTask->function = function;
    newTask->arg = arg;
    newTask->next = NULL;

    //pthread_mutex_lock(&pool->queueMutex);

    if (pool->taskQueueTail) {
        pool->taskQueueTail->next = newTask;
    } else {
        pool->taskQueueHead = newTask;
    }
    pool->taskQueueTail = newTask;

    pool->queueSize++; // Creștem dimensiunea cozii

    pthread_cond_signal(&pool->condition);
    pthread_mutex_unlock(&pool->queueMutex);
}

// Distrugere ThreadPool
void threadPoolDestroy(ThreadPool *pool) {
    if (!pool) return;

    pthread_mutex_lock(&pool->queueMutex);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->condition);
    pthread_mutex_unlock(&pool->queueMutex);

    for (size_t i = 0; i < pool->threadCount; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);

    // Golire coadă de task-uri
    while (pool->taskQueueHead) {
        Task *task = pool->taskQueueHead;
        pool->taskQueueHead = pool->taskQueueHead->next;
        free(task);
    }

    pthread_mutex_destroy(&pool->queueMutex);
    pthread_cond_destroy(&pool->condition);
    free(pool);
}
