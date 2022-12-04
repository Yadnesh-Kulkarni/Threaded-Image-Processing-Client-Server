#include "../Utils/include/csapp.h"
#include "../Utils/include/cvutils.h"

#define HOST "127.0.0.1"
#define PORT 8080

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("Usage : ./client <Image Path> <Operation>\n");
        return 1;
    }

    int clientfd,read_size;
    char buf[10240];
    FILE *fp;
    rio_t rio;
    int operation = atoi(argv[2]);
    if(operation > 4 || operation < 1)
    {
        printf("Invalid operation : Refer to the list below\n1. Grayscale Conversion\n2. Color Inversion\n3. Blur\n4. Create Border\n");
        return 1;
    }
    // Open file to see if it exists
    fp = Fopen(argv[1], "r");
    if(fp == NULL) // File does not exist , return
    {
        printf("Image file not found... Exiting \n");
        return 1;
    }

    // Get File Size
    fseek(fp, 0 , SEEK_END);
    ssize_t size = ftell(fp);
    fseek(fp , 0 , SEEK_SET);
    // End get file size

    // Open connection to HOST at PORT
    clientfd = Open_clientfd(HOST, PORT); 

    // Initiate Object for Robust I/O
    Rio_readinitb(&rio, clientfd); 

    // Sending File Size to Server
    Rio_writen(clientfd, &size, sizeof(size));

    // Sending operation to server
    Rio_writen(clientfd, &operation, sizeof(operation));

    // Send file data to server 
    while(!feof(fp)) {
        read_size = Fread(buf, 1, MAXLINE, fp);
        Rio_writen(clientfd, buf, read_size);
        bzero(buf, sizeof(buf));
    }
    // Closing file
    Fclose(fp);

    // Done Sending File Data to Server
    
    // ====================================================================================================================

    // Receive Data From Server
    ssize_t receiveFileSize;

    // Receive File Size From Server
	Rio_readnb(&rio,&receiveFileSize,sizeof(receiveFileSize));
    
    // Get Data From Server
    int n;
    char *data = NULL;
    ssize_t totalSize = 0;
    int receiveBufferSize = MAXLINE;

    // Receive file from Server
    while((n = Rio_readnb(&rio, buf, receiveBufferSize)) > 0 && receiveFileSize > 0){
        data = realloc(data, totalSize + n);
        memcpy(data + totalSize, buf, n);
        totalSize = totalSize + n;
        receiveFileSize -= n;
        if(receiveFileSize < receiveBufferSize) // If remaining data is less that data size then only read remaining data
        {
            receiveBufferSize = receiveFileSize;
        }
    }
    Close(clientfd);

    // View the original image
    viewImageWithPath(argv[1]);

    // View the received image
    viewImage(data, totalSize);

    if(data)
    {
        free(data);
    }
    exit(0);
}
