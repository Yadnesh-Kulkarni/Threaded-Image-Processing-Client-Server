#include "server.h"
#include <pthread.h>

#define PORT 8080
#define MAX_THREADS 3
// Function Declarations
void* getfiledata(int, char*, size_t*, size_t, rio_t*);
void processconnection(int, pthread_t);
static void *thread_start(void*);

// Global array which holds handles for all threads
pthread_t threadArray[MAX_THREADS];

int main()
{
    int listenfd, connfd;
    pthread_t tid;
    queue *q;
    q = qInit();

    // Start worker thread pool
    int i;
    for(i = 0 ; i < MAX_THREADS; i++)
    {
        pthread_create(&threadArray[i],NULL, thread_start, q);
    }

    listenfd = Open_listenfd(PORT);
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
            printf("Main Thread Says : Queue is full\n");
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

    Sleep(10);
    // Run infinitely
    while(1)
    {
        // Grab lock so that race conditions can be prevented for queue
        pthread_mutex_lock(q->qMutex);

        // Once locked, check whether queue is empty
        while(isQEmpty(q))
        {
            // If it is empty, then wait till main thread writes a new request on buffer
            printf("%ld Says : Queue is currently empty...\n", tid);
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
    // char client_hostname[MAXLINE], client_port[MAXLINE];
    // getnameinfo((SA *)&(conninf->clientAddr), conninf->clientLen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
    // printf("Connected to (%s, %s)\n", client_hostname, client_port);
}

void processconnection(int connfd, pthread_t tid)
{
    char* data = NULL;
    char buf[MAXLINE];
    rio_t rio;

    char pathToSave[256]; 

    sprintf(pathToSave, "./Gray_Image_%ld.jpg", tid);

    Rio_readinitb(&rio, connfd);
    
    // Get File Size
    ssize_t receiveFileSize;
    int receiveDataSize = MAXLINE;
	Rio_readnb(&rio,&receiveFileSize,sizeof(receiveFileSize));
    
   
    // Get Data From Client
    size_t totalSize = 0;
    size_t n = 0;
    while((n = Rio_readnb(&rio, buf, receiveDataSize)) > 0 && receiveFileSize > 0){
        data = realloc(data, totalSize + n);
        memcpy(data + totalSize, buf, n);
        totalSize = totalSize + n;
        receiveFileSize -= n;
        if(receiveFileSize < receiveDataSize)
        {
            receiveDataSize = receiveFileSize;
        }
    }


    // ====================================== DATA RECEIVE FINISHED =====================================================

    // Sending Image for Conversion
    // The function will also save a copy of image that can be read for further process
    int res = convertImageDataToGrayScale(data, &totalSize, pathToSave);
    if(data) // Free the buffer
    {
        free(data);
    }
    if(res == -1) // If Image conversion was unsuccessful then exit this thread // Need to send exit condition to client, filesize = 0
    {
        Rio_writen(connfd, 0, 4);
        return;
    }

    // Send converted image back to client
    FILE* fp;
    fp = Fopen(pathToSave, "r");
    if(fp == NULL)
    {
        printf("Gray Scale Image file not found... Exiting \n");
        Rio_writen(connfd, 0, 4);
        return;
    }
    // Get File Size
    fseek(fp, 0 , SEEK_END);
    ssize_t sendFileSize = ftell(fp);
    fseek(fp , 0 , SEEK_SET);
 

    // Send File Size to Client
    Rio_writen(connfd, &sendFileSize, sizeof(sendFileSize));

    // Sending Data to Client
    size_t read_size = 0;
    while(!feof(fp)) {
        read_size = Fread(buf, 1, MAXLINE, fp);
        Rio_writen(connfd, buf, read_size);
        bzero(buf, sizeof(buf));
    }

    // Close File
    Fclose(fp);

    // Remove File as it is no longer required
    remove(pathToSave);
}

