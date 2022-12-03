#include "../Utils/include/csapp.h"
#include "../Utils/include/cvutils.h"
#include <pthread.h>

#define PORT 8080

void* getfiledata(int, char*, size_t*, size_t, rio_t*);
void processconnection(int, pthread_t);
static void *thread_start(void*);

struct ConnectionInfo{
    int connfd;
    struct sockaddr_storage clientAddr;
    socklen_t clientLen;
};



int main()
{
    int listenfd, connfd;
    pthread_t tid;

    listenfd = Open_listenfd(PORT);
    while(1){
        struct ConnectionInfo *conninf = (struct ConnectionInfo *)Malloc(sizeof(struct ConnectionInfo));
        conninf->clientLen = sizeof(struct sockaddr_storage);
        conninf->connfd = Accept(listenfd, (SA *)&(conninf->clientAddr), &(conninf->clientLen));
        Pthread_create(&tid,NULL, &thread_start, (void *)conninf);
    }
    exit(0);
}

static void *thread_start(void* arg)
{
    char client_hostname[MAXLINE], client_port[MAXLINE];
    struct ConnectionInfo *conninf = (struct ConnectionInfo *)arg;
    getnameinfo((SA *)&(conninf->clientAddr), conninf->clientLen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
    printf("Connected to (%s, %s)\n", client_hostname, client_port);
    pthread_t tid;
    tid = pthread_self();
    processconnection(conninf->connfd, tid);
    Close(conninf->connfd);
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

