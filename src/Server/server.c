#include "../../include/Server/server.h"
#include <pthread.h>

// Global array which holds handles for all threads
pthread_t threadArray[MAX_THREADS];

// Barrier
pthread_barrier_t barrier;

int main(int argc, char **argv)
{
    int listenfd;
    pthread_t tid;
    queue *q;
    q = qInit();

    // Setting up thread priorities
    struct sched_param param;
    pthread_attr_t lowPriorityAttr;
    int mainThreadPriority;

    // // Main thread high priority
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    mainThreadPriority = param.sched_priority;

    // Worker threads with low priority
    pthread_attr_init(&lowPriorityAttr); // Init with default parameters
    pthread_attr_setinheritsched(&lowPriorityAttr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&lowPriorityAttr, SCHED_FIFO);

    param.sched_priority = mainThreadPriority - 1;
    if(pthread_attr_setschedparam(&lowPriorityAttr,&param))
    {
        printf("Unable to set priority for thread...\n");
    }

    // Done setting up thread priorities

    // Start worker thread pool
    int i;
    for(i = 0 ; i < MAX_THREADS; i++)
    {
        Pthread_create(&threadArray[i],&lowPriorityAttr, thread_start, q);
    }

    // Start listening
    listenfd = Open_listenfd(PORT);
    if(listenfd < 0)
    {
        printf("Server could not start Listen FD... Exiting\n");
        for(i = 0 ; i < MAX_THREADS; i++)
        {
            Pthread_cancel(threadArray[i]);
        }
        qDestroy(q);
        return 0;
    }

    while(1){
        // Allocate a ConnectionInfo struct
        ConnectionInfo *conninf = (ConnectionInfo *)Malloc(sizeof(ConnectionInfo));
        conninf->clientLen = sizeof(struct sockaddr_storage);
        conninf->connfd = Accept(listenfd, (SA *)&(conninf->clientAddr), &(conninf->clientLen));
        
        // Grab the lock so that no one else can access queue
        pthread_mutex_lock(q->qMutex);
        qPush(q, conninf);

        // Check whether the queue is full or not
        while(isQFull(q))
        {
            // If queue is full then print a message and wait
            // Wait until some thread picks up a request from buffer and buffer is not full
            printf("Main Thread Says : Queue is full : Priority - %d\n", mainThreadPriority);
            pthread_cond_wait(q->notFull, q->qMutex);
        }
        // Unlock the Mutex which was grabbed in pthread_cond_wait
        pthread_mutex_unlock(q->qMutex);

        // Signal worker threads that a new request has been added to the buffer
        pthread_cond_signal(q->notEmpty);
    }

    // Not that we will reach here
    qDestroy(q);
    exit(0);
}

static void *thread_start(void* arg)
{
    // Get queue which was passed as an argument
    queue* q = (queue *)arg;
    pthread_t tid = pthread_self();
    struct sched_param param;
    int currentThreadPriority, policy;
    pthread_getschedparam(tid, &policy, &param);
    currentThreadPriority = param.sched_priority;

    // Run infinitely
    while(1)
    {
        printf("Started processing by thread %ld\n", tid);
        delay(30);
        printf("End processing by thread %ld\n", tid);

        // Grab lock so that race conditions can be prevented for queue
        pthread_mutex_lock(q->qMutex);

        // Once locked, check whether queue is empty
        while(isQEmpty(q))
        {
            // If it is empty, then wait till main thread writes a new request on buffer
            printf("Worker thread %ld Says : Queue is currently empty : Priority - %d\n", tid, currentThreadPriority);
            pthread_cond_wait(q->notEmpty, q->qMutex);
        } 

        // We are here, it means queue is not empty and can be read
        ConnectionInfo *conninf = qPop(q);

        // Unlock the mutex
        pthread_mutex_unlock(q->qMutex);

        // Signal anyone waiting to on notFull
        pthread_cond_signal(q->notFull);

        // Process the client
        processconnection(conninf->connfd, tid);

        // Close connected file descriptor
        Close(conninf->connfd);

        // Free the data
        if(conninf)
        {
            free(conninf);
        }
    }
}
 