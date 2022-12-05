#include "../Utils/csapp.h"
#include "../Utils/cvutils.h"

#define PORT 8080
#define MAX_THREADS 3
#define MAX_Q_SIZE MAX_THREADS  * 2

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
void qDestroy(queue *q);
int qPush(queue *q, ConnectionInfo *newConnection);
ConnectionInfo* qPop(queue *q);
int isQEmpty(queue *q);
int isQFull(queue *q);

// Utility Functions
// Function Declarations
void* getfiledata(int, char*, size_t*, size_t, rio_t*);
void processconnection(int, pthread_t);
static void *thread_start(void*);
void delay(int);




