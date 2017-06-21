#include <stdio.h>      // I/O stream
#include <stdlib.h>     // Use some special function
#include <string.h>     // String processing function
#include <errno.h>      // Error number
#include <signal.h>     // signal()
#include <unistd.h>     // Provide functions for file and directory operations
#include <sys/types.h>
#include <sys/socket.h> // socket()
#include <netinet/in.h> // Provide the structure of sockaddr_in
#include <arpa/inet.h>
#include <sys/wait.h>   // wait()
#include <pthread.h>
#include <sys/stat.h>
#include <time.h>


#include "ContentType.h"


// Used to output error messages
void PANIC(char *msg);
#define PANIC(msg){perror(msg);exit(-1);}

#define GET_COMMON 101
#define BAD_REQUEST 102
#define POST 103

typedef struct HttpRequest{
    char method[20];    /* The method of request */
    char *path;         /* The file path */
    char *content;      /* The file content */
    int contentlength;  /* The length of content */
    int contenttype;    /* The type of content */
    int rangeflag;      /* Flag */
    long rangestart;    /* The starting position of the data */
    long rangeend;      /* The end of the data */
    long rangetotal;    /* The total length of the data */
    char prefix[20];    /* The filename extension */
    int responsecode;   /* State code */
}HR;

void* threadfunction(void *thisParam);
int getrequest(char *requestbuf, struct HttpRequest *httprequest);
void getcommon(int clientfd, struct HttpRequest *httprequest);
int transferfile(int clientfd, FILE *fp, int type, 
                 int rangestart, int rangetotal);
int senddata(int clientfd, char *buf, int length);
void responsecode(int clientfd, int code, struct HttpRequest *httprequest);
// char* getcontenttype(char *type);
// char* getcurrenttime();

int main(int argc, char *argv[]){
    int port;    // port number
    int sockfd;    // return from socket()
    int len;    // sizeof(client_addr)
    int new_fd; // return form accept()
    int yes=1;  // For setsockopt()
    pthread_t threadid;
    struct sockaddr_in server_addr, client_addr;

    // Used to handle signal pipe
    struct sigaction action;
    action.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &action, 0);

    // Used to handle signal child
    __sighandler_t prehandler;
    prehandler = signal(SIGCHLD, SIG_IGN);
    if(prehandler == SIG_ERR) PANIC("Signal");

    // Set port number
    if(argc>1) port = atoi(argv[1]);
    else port = 80;

    // Create a socket
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd<0) PANIC("Socket");

    // We must release the port which we used before.
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
        PANIC("setsockopt");

    server_addr.sin_family = AF_INET;           // IPv4
    server_addr.sin_port = htons(port);         // Set port
    server_addr.sin_addr.s_addr = INADDR_ANY;    // 0.0.0.0

    // Used to bind sockfd
    if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0)
        PANIC("Bind");

    // Monitor sockfd. Up to twenty connections
    if(listen(sockfd, 20) != 0)
        PANIC("Listen");

    // Infinite Loop
    while(1){
        /* Configure a memory to place new_fd */
        int *thisParam = calloc(1, sizeof(int));
        len = sizeof(client_addr);

        // Wait for connection from client
        new_fd = accept(sockfd, 
                        (struct sockaddr*) &client_addr, 
                        (socklen_t*)&len);
        if(new_fd<0) PANIC("Accept");

        printf("Client from %s : %d\n", 
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port));

        /* Put new_fd into the memory space which we configured.
         * It will avoid some mistakes.
         */
        *thisParam = new_fd;

        /* Create a thread */
        if(pthread_create(&threadid, NULL, threadfunction, thisParam) != 0){
            PANIC("pthread");
        } else{
            /* After finishing the thread function, 
             * the resource will automatically freed
             */
            pthread_detach(threadid);
        }
    }

    close(sockfd);
    return 0;
}

/* Used to slove the request of client */
void* threadfunction(void *thisParam){
    char buffer[1024];
    int clientfd = *(int*)thisParam; /* Get client's fd from our memory space */
    free(thisParam); /* Release the memory space */
    int recvnum = 0;  /* Record the data where we read */
    int Data = 0;
    struct HttpRequest httprequest;
    httprequest.content = NULL;
    httprequest.path = NULL;
    httprequest.path = (char*) malloc(1024);
    httprequest.rangeflag = 0;
    httprequest.rangestart = 0;

    /* Use while loop to confirm that the data has been delivered */
    while(1){
        Data = recv(clientfd, buffer+recvnum, sizeof(buffer)-recvnum-1, 0);
        if(Data<=0){
            close(clientfd);
            pthread_exit(NULL);
            PANIC("recv");
        }

        recvnum += Data;
        buffer[recvnum] = '\0';

        /*If we receive "\r\n\r\n" or "\n\n" , we can break.*/
        if(strstr(buffer, "\r\n\r\n") != NULL || strstr(buffer, "\n\n") != NULL)
            break;
    }

    printf("Request: \n%s\n", buffer);  /* Just print the request content */

    /* After receving the request, we must resolve the information */
    switch(getrequest(buffer, &httprequest)){
        case GET_COMMON:
            getcommon(clientfd, &httprequest);
            break;
        default:
            break;
    }

    /* Remember to release memory space. */
    if(httprequest.path != NULL)
        free(httprequest.path);

    // if(httprequest.content != NULL)
    //     free(httprequest.content);

    close(clientfd);
    return 0;
}

/* Used to resolve the request of client */
int getrequest(char *requestbuf, struct HttpRequest *httprequest){
    char protocol[20];
    char prefix[20];
    char path[200];
    int i, j, k=0;

    /* If the first line is less than 3 elements, 
     * it means the request have some problem.
     */
    if(sscanf(requestbuf, "%s %s %s", httprequest->method, path, protocol) != 3)
        return BAD_REQUEST;

    /* If the request method is "GET"... */
    if(strcmp(httprequest->method, "GET") == 0){
        if(path[strlen(path)-1] == '/'){
            /* Link string */
            strcat(path, "test.html");
        }

        // sprintf(httprequest->path, "%s", path);

        /*
        for(i=0; i<strlen(path); i++){
            if(path[i]=='/'){
                j; k=0;
                for(j=i+1; j<strlen(path); j++){
                    filename[k] = path[j];
                    k++;
                }
                break;
            }
        }
        sprintf(httprequest->path, "%s", filename);
        */

        if(sscanf(path, "/%s.%s", httprequest->path, httprequest->prefix) != 2)
            strcpy(httprequest->prefix, "*");

        for(i=0; i<strlen(path); i++){
            if(path[i]=='.'){
                for(j=i+1; j<strlen(path); j++){
                    prefix[k] = path[j];
                    k++;
                }
                break;
            }
        }
        sprintf(httprequest->prefix, "%s", prefix);

        printf("Path: %s\nPrefix: %s\n", 
               httprequest->path, 
               httprequest->prefix);

        return GET_COMMON;
    }

    return -1;
}

/* Used to handle GET request */
void getcommon(int clientfd, struct HttpRequest *httprequest){
    struct stat s;
    FILE *fp = fopen(httprequest->path, "r");

    /* If file not exists */
    if(fp == NULL){
        printf("file not exist\n");
        responsecode(clientfd, 404, httprequest);
    }

    /* If file exists */
    else{
        puts("file exists");
        if(httprequest->rangeflag == 0){
            stat(httprequest->path, &s);
            httprequest->rangetotal = s.st_size;  /* Get Data length */

            printf("totallength: %ld\n\n\n", httprequest->rangetotal);
        }

        /* Used to send HTTP response */
        responsecode(clientfd, 200, httprequest);

        /* Used to Send the file fp which client request. */
        transferfile(clientfd, fp, httprequest->rangeflag, 
                     httprequest->rangestart, httprequest->rangetotal);

        // puts("flag1");

        fclose(fp);  /* Close the file */
    }
}

/* Used to send HTTP response */
void responsecode(int clientfd, int code, struct HttpRequest *httprequest){
    char buffer[2048];
    char content[1023];

    httprequest->responsecode = code;

    switch(code){
        case 200:
            sprintf(buffer,
                    "HTTP/1.1 200 OK\r\n"
                    "Server: WenYuan/1.0\r\n"
                    "Content-Type: %s\r\n"
                    "Accept-Ranges: bytes\r\n"
                    "Content-Length: %ld\r\n"
                    "Connection: close\r\n"
                    "\r\n", 
                    getContentType(httprequest->prefix), 
                    httprequest->rangetotal);
            break;

        case 404:
            strcpy(content, 
                   "<html><head><title>404 Not Found</title></head>"
                   "<body><h1>404 Not Found</h1>"
                   "File Not Found.</body></html>");
            sprintf(buffer,
                    "HTTP/1.1 404 Object Not Found\r\n"
                    "Server: WenYuan/1.0\r\n"
                    "Content-Type: %s\r\n"
                    "Content-Length: %ld\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "%s", 
                    "text/plain", strlen(content), content);
            break;

        default:
            break;
    }

    /* Send the response to client. */
    senddata(clientfd, content, strlen(content));
}

/*
char* getcurrenttime(){
    time_t timep;
    char* localvalue;
    time(&timep);
    sprintf(localvalue, "%s", asctime(gmtime(&timep)));
    return localvalue;
}
*/

/* To get data type of the file which client need. */
/*
char* getcontenttype(char *type){
    char* ContentType[16]={
        "jpeg", "image/jpeg",
        "png", "image/png",
        "gif", "image/gif",
        "jpg", "image/jpeg",
        "txt", "text/plain",
        "htm", "text/html",
        "html", "text/html",
        "*", "application/octet-stream"
    };

    int nType = sizeof(ContentType)/16;
    int i;
    for(i=0; i<nType; i+=2){
        if(strcmp(ContentType[i], type) == 0)
            return ContentType[i+1];
    }
    return "application/octet-stream";
}
*/
/* Used to transfer file. */
int transferfile(int clientfd, FILE *fp, int type, 
                 int rangestart, int totallength){
    if(type == 1) fseek(fp, rangestart, 0);

    int sendnum = 0;
    int segment = 1024;

    /* 
     * feof(fp)-->If the data has not been read completely, 
     * it will return zero.
     * 
     * To read the data from the file fp.
     */
    while(!feof(fp) && sendnum < totallength){
        char buf[segment];
        memset(buf, 0, 1024);
        int i = 0;

        while(!feof(fp) && i < segment && sendnum+i < totallength){
            buf[i++] = fgetc(fp);
        }

        /* Send data every 1024 bytes */

        if(senddata(clientfd, buf, i) == 0)
            return 0;

        sendnum += i;
    }

    return 1;
}

/* Used to transfer file. */
int senddata(int clientfd, char *buf, int length){
    if(length <= 0)
        return 0;

    int result = send(clientfd, buf, length, 0);
    if(result < 0)
        return 0;

    return 1;
}
