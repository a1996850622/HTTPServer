#include "HTTPServer.h"

int main(int argc, char *argv[]){
    int port = DEFAULT_PORT;    // Server port
    int sockfd;                 // return from socket()
    int yes = 1;                // For setsockopt()
    struct sockaddr_in server_addr;

    struct sockaddr_in client_addr;
    int len;                    // sizeof(client_addr)
    int clientfd; 
    
    pthread_t threadid;

    // Used to handle signal pipe
    struct sigaction action;
    action.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &action, 0);

    // Used to handle signal child, *fork*
    // __sighandler_t prehandler;
    // prehandler = signal(SIGCHLD, SIG_IGN);
    // if(prehandler == SIG_ERR) PANIC("Signal");

    // Args define port witch be used.
    if(argc > 1){
        int argsBuff = atoi(argv[1]);
        if(argsBuff <= 0 || argsBuff > 65535) perror("Args");
        else port = argsBuff;
    }
    // printf("Port: %d\n", port);
    
    
    // Create a socket.
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) PANIC("Socket()");
    
    // Release the port which used before.
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
        PANIC("setsockopt()");

    server_addr.sin_family = AF_INET;           // IPv4
    server_addr.sin_port = htons(port);         // Set port
    server_addr.sin_addr.s_addr = INADDR_ANY;   // 0.0.0.0
    
    // Bind.
    if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0)
        PANIC("Bind");
    
    // Listen sockfd. Up to 20 connections.
    if(listen(sockfd, 20) != 0)
        PANIC("Listen");
    
    // Server Loop.
    while(1){
        // Configure a memory to place new_fd.
        int *threadArgs = calloc(1, sizeof(int));
        
        len = sizeof(client_addr);
        
        // Wait for connection from client...
        
        // Accept.
        clientfd = accept(sockfd, (struct sockaddr*) &client_addr, 
                          (socklen_t*) &len);
        if(clientfd < 0) PANIC("Accept");
        
        printf("Client from %s : %d\n", 
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port));
        
        *threadArgs = clientfd;
        
        // Create a thread.
        if(pthread_create(&threadid, NULL, threadFunc, threadArgs) != 0){
            PANIC("pthread_create()");
        } else{
            // After finishing the thread function, 
            // the resource will automatically freed.
            pthread_detach(threadid);
        }

    }   // Server Loop. While()

    close(sockfd);
    return 0;
}   // main()

void* threadFunc(void *threadArgs){
    char recvBuffer[RECEIVE_BUFFER_SIZE];
    int clientfd = *(int*) threadArgs;
    free(threadArgs);
    
    int recvPt = 0;
    int recvSize = 0;

    struct HttpRequest httpRequest;
    httpRequest.content = NULL;
    httpRequest.path = (char*) malloc(PARSE_BUFFER_SIZE);
    httpRequest.prefix = (char*) malloc(PARSE_BUFFER_SIZE);
    httpRequest.rangeflag = 0;
    httpRequest.rangestart = 0;
    
    // Receive loop.
    while(1){
        recvSize = recv(clientfd, recvBuffer+recvPt, 
        sizeof(recvBuffer) - recvPt - 1, 0);
        if(recvSize <= 0){
            close(clientfd);
            pthread_exit(NULL);
            PANIC("recv()");
        }
        recvPt += recvSize;
        recvBuffer[recvPt] = '\0';
        
        // When received "\r\n\r\n" or "\n\n" , break.
        if(strstr(recvBuffer, "\r\n\r\n") != NULL || 
           strstr(recvBuffer, "\n\n") != NULL)
            break;
    } // Receive loop.
    
    printf("Request: \n%s\n", recvBuffer); 

    // Parse request.
    switch(getRequest(recvBuffer, &httpRequest)){
        case GET_COMMAND:
            getCommand(clientfd, &httpRequest);
            break;
        default:
            break;
    }

    // Release memory space.
    // puts("FLAG1");
    // if(httpRequest.path != NULL)
    //     free(httpRequest.path);
    // puts("FLAG2");
    // if(httpRequest.prefix != NULL)
    //     free(httpRequest.prefix);

    // if(httpRequest.content != NULL)
    //     freehttpRequest.content);

    close(clientfd);
    return 0;
    
}   // threadFunc()

int getRequest(char *requestbuf, struct HttpRequest *httpRequest){
    char path[PARSE_BUFFER_SIZE];
    char protocol[20];

    // Parse Http Header.
    if(sscanf(requestbuf, "%s %s %s", 
              httpRequest -> method, path, protocol) != 3)
        return BAD_REQUEST;

    // Method "GET"
    if(strcmp(httpRequest->method, "GET") == 0){
        if(path[strlen(path)-1] == '/') strcat(path, "test.html");
        
        httpRequest -> path = path;
        char *base = basename(path);

        char *ext = strrchr(base,'.');
        if(!ext) strcpy(httpRequest -> prefix, "*");
        else {
            ext = ext + 1; 
            strcpy(httpRequest -> prefix, ext);
        }

        printf("Path: %s\n"
            "Prefix: %s\n", 
            httpRequest -> path, 
            httpRequest -> prefix);
    
        return GET_COMMAND;
    } // Method "GET"
    
    return -1;
} // getRequest()

void getCommand(int clientfd, struct HttpRequest *httpRequest){
    struct stat s;
    char path_R[PARSE_BUFFER_SIZE];
    char path[PARSE_BUFFER_SIZE];
    strcpy(path_R, httpRequest -> path);
    sprintf(path, "%s%s", "./www/",path_R + 1);
    printf("fopen() Path: %s\n", path);
    
    
    FILE *fp = fopen(path, "r");
    
    // If file does not exists.
    if(fp == NULL){
        printf("File not exist: %s\n", path);
        responseCode(clientfd, 404, httpRequest);

    } 
    // If file exists.
    else {
        printf("File exist: %s\n", path); // puts("file exists");
        
        if(httpRequest -> rangeflag == 0){
            stat(path, &s);
            httpRequest -> rangetotal = s.st_size;
            printf("total length: %ld\n\n\n", httpRequest -> rangetotal);
        }
        
        responseCode(clientfd, 200, httpRequest);
        
        transferfile(clientfd, fp, httpRequest -> rangeflag, 
                     httpRequest -> rangestart, httpRequest -> rangetotal);
        
        fclose(fp);
    } // File exists.
}

// Send HTTP response.
void responseCode(int clientfd, int code, struct HttpRequest *httpRequest){
    char sendBuffer[SEND_BUFFER_SIZE];
    char content[CONTENT_BUFFER_SIZE];
    
    httpRequest -> responsecode = code;
    
    switch(code){
        case 200:
            sprintf(sendBuffer,
                    "HTTP/1.1 200 OK\r\n"
                    "Server: WenYuan/1.0\r\n"
                    "Content-Type: %s\r\n"
                    "Accept-Ranges: bytes\r\n"
                    "Content-Length: %ld\r\n"
                    "Connection: close\r\n"
                    "\r\n", 
                    getContentType(httpRequest->prefix), 
                    httpRequest->rangetotal);
            break;

        case 404:
            strcpy(content, 
                   "<!DOCTYPE html>"
                   "<html><head><title>404 Not Found</title></head>"
                   "<body><h1>404 Not Found</h1>"
                   "File Not Found.</body></html>");
            sprintf(sendBuffer,
                    "HTTP/1.1 404 Object Not Found\r\n"
                    "Server: WenYuan/1.0\r\n"
                    "Content-Type: %s\r\n"
                    "Content-Length: %ld\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "%s", 
                    getContentType("html"), strlen(content), content);
            break;

        default:
            break;
    }
    
    // Send the response to client.
    sendData(clientfd, sendBuffer, strlen(sendBuffer));
    
}

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

        //  Send data every 1024 bytes
        if(sendData(clientfd, buf, i) == 0)
            return 0;

        sendnum += i;
    }

    return 1;
}


// Used to transfer file.
int sendData(int clientfd, char *buf, int length){
    if(length <= 0)
        return 0;

    int result = send(clientfd, buf, length, 0);
    if(result < 0)
        return 0;

    return 1;
}
