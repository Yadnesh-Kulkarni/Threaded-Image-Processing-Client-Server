#include "server.h"

queue* qInit()
{
    queue* q = (queue *) Malloc(sizeof(queue));
    if(q == NULL) return NULL;

    // Initialize Queue with default parameters
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;

    // Initialize Mutex
    q->qMutex = (pthread_mutex_t *) Malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(q->qMutex, NULL);

    // Initialize Conditional Variable if it is full (Main thread could wait on this)
    q->notFull = (pthread_cond_t *) Malloc(sizeof(pthread_cond_t));
    pthread_cond_init(q->notFull, NULL);

    // Initialize Conditional Variable if it is empty (Worker threads could wait on this)
    q->notEmpty = (pthread_cond_t *) Malloc(sizeof(pthread_cond_t));
    pthread_cond_init(q->notEmpty, NULL);

    return q;
}

// Destroys the queue object
void qDestroy(queue *q)
{
    // Destroy Conditional Variable
    pthread_cond_destroy(q->notEmpty);
    free(q->notEmpty);

    // Destroy Conditional Variable
    pthread_cond_destroy(q->notFull);
    free(q->notFull);

    // Destroy Mutex
    pthread_mutex_destroy(q->qMutex);
    free(q->qMutex);

    if(q->size > 0)
    {
        ConnectionInfo* temp = qPop(q);
        if(temp)
        {
            free(temp);
        }
    }

    free(q);
}


// Inserts new element ConnectionInfo into the queue.
// Assumes that incoming newConnection already has memory allocated
// Returns 1 if push was successful, 0 otherwise
int qPush(queue *q, ConnectionInfo *newConnection)
{
    if(isQFull(q))
    {
        return 0;
    }

    
    queueNode* qNode = (queueNode *)Malloc(sizeof(queueNode));
    qNode->connInf = newConnection;
    qNode->next = NULL;
    // Q is Empty then both head and tail will be at new Node
    if(isQEmpty(q))
    {
        q->head = qNode;
        q->tail = qNode;
    }
    else // Else new connection is added at the end and tail is moved ahead
    {
        q->tail->next = qNode;
        q->tail = qNode;
    }
    q->size++;
    return 1;
}

// Returns element at the front of the queue
ConnectionInfo* qPop(queue* q)
{
    if(isQEmpty(q))
    {
        printf("Queue is empty\n");
        return NULL;
    }

    // Get element at head and move head to next pointer
    ConnectionInfo* connInfo = q->head->connInf;
    q->head = q->head->next;
    q->size--;
    return connInfo;
}


// Returns 1 if queue is empty, 0 otherwise
int isQEmpty(queue* q)
{
    if(q->size == 0)
    {
        return 1;
    }
    return 0;
}

// Returns 1 if queue is full(at max capacity), 0 otherwise
int isQFull(queue* q)
{
    if(q->size == MAX_Q_SIZE)
    {
        return 1;
    }
    return 0;
}





