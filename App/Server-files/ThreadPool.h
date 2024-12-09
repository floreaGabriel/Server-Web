#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <stddef.h>

// Structura task-ului din coadă
typedef struct Task {
    void (*function)(void *); // Funcția de executat
    void *arg;                // Argumentul funcției
    struct Task *next;        // Următorul task
} Task;

// Structura threadpool-ului
typedef struct ThreadPool {
    pthread_t *threads;         // Array de thread-uri
    Task *taskQueueHead;        // Capul cozii de task-uri
    Task *taskQueueTail;        // Coada cozii de task-uri
    pthread_mutex_t queueMutex; // Mutex pentru coadă
    pthread_cond_t condition;   // Condiție pentru sincronizare
    int stop;                   // Flag pentru oprire
    size_t threadCount;         // Numărul de thread-uri
    size_t queueSize; //cati sunt actual

} ThreadPool;

// Funcții pentru utilizarea threadpool-ului
ThreadPool *threadPoolCreate(size_t numThreads);
void threadPoolEnqueue(ThreadPool *pool, void (*function)(void *), void *arg);
void threadPoolDestroy(ThreadPool *pool);

#endif // THREADPOOL_H