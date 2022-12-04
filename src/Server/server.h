#include "../Utils/include/csapp.h"
#include "../Utils/include/cvutils.h"

#define MAX_Q_SIZE 6

// Structure to hold connection info of client
typedef struct ConnectionInfo{
    int connfd;
    struct sockaddr_storage clientAddr;
    socklen_t clientLen;
}ConnectionInfo;

// Structure representing single node in the queue
typedef struct queueNode{
    ConnectionInfo *connInf;
    struct queueNode *next;
}queueNode;


// Structure representing queue
typedef struct queue{
    queueNode *head;
    queueNode *tail;
    int size;
    pthread_mutex_t *qMutex;
	pthread_cond_t *notFull, *notEmpty;
}queue;

// Queue Functions
queue* qInit();
int qPush(queue *q, ConnectionInfo *newConnection);
ConnectionInfo* qPop(queue *q);
int isQEmpty(queue *q);
int isQFull(queue *q);




