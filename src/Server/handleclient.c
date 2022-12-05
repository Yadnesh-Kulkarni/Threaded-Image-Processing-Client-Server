#include "../../include/Server/server.h"

void processconnection(int connfd, pthread_t tid)
{
    char* data = NULL;
    char buf[MAXLINE];
    rio_t rio;

    char pathToSave[256]; 

    sprintf(pathToSave, "./images/Processed_Image_%ld.jpg", tid);

    Rio_readinitb(&rio, connfd);
    
    // Get File Size
    ssize_t receiveFileSize;
    int receiveDataSize = MAXLINE;
	Rio_readnb(&rio,&receiveFileSize,sizeof(receiveFileSize));
    
   
    // Get Operation Type
    int operation;
    Rio_readnb(&rio, &operation, sizeof(operation));

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
    int res = -1;
    switch (operation)
    {
    case 1: res = convertImageDataToGrayScale(data, &totalSize, pathToSave);
            break;
    case 2: res = convertImageDataToInverted(data,&totalSize,pathToSave);
            break;
    case 3: res = convertImageDataToBlur(data,&totalSize,pathToSave);
            break;
    case 4: res = convertImageDataToBorderedImage(data,&totalSize,pathToSave);
            break;
    default:
        break;
    }

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

void delay(int t)
{
    struct timeval start, end;
    time_t start_time, cur_time;

    time(&start_time);
    do
    {
        time(&cur_time);
    }while((cur_time - start_time) < t);

} 