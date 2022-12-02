#include "../Utils/include/csapp.h"
#include "../Utils/include/cvutils.h"

#define PORT 8080

void* getfiledata(int, char*, size_t*, size_t, rio_t*);
void processconnection(int connfd);

int main()
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], clien_port[MAXLINE];

    listenfd = Open_listenfd(PORT);
    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        // This is where threads should begin executing
        getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, clien_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, clien_port);
        processconnection(connfd);
        printf("(%s, %s) Disconnected\nWaiting for new Connection\n", client_hostname, clien_port);
        fflush(stdout);
        Close(connfd);
    }
    exit(0);
}

void processconnection(int connfd)
{
    char* data = NULL;
    char buf[MAXLINE];
    rio_t rio;

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
    int res = convertImageDataToGrayScale(data, &totalSize);
    if(data) // Free the buffer
    {
        free(data);
    }
    if(res == -1) // If Image conversion was unsuccessful then exit this thread // Need to send exit condition to client, filesize = 0
    {
        return;
    }

    // Send converted image back to client
    FILE* fp;
    fp = Fopen("Gray_Image.jpg", "r");
    if(fp == NULL)
    {
        printf("Gray Scale Image file not found... Exiting \n");
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
    remove("Gray_Image.jpg");
}

