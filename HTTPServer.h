#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

void PANIC(char *msg);
#define PANIC(msg){perror(msg);exit(-1);}

#define DEFAULT_PORT 80
#define RECEIVE_BUFFER_SIZE 1024
#define SEND_BUFFER_SIZE 2048
#define CONTENT_BUFFER_SIZE 1024
#define PARSE_BUFFER_SIZE 1024

#define GET_COMMAND 101
#define BAD_REQUEST 901


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
    char *prefix;       /* The filename extension */
    int responsecode;   /* State code */
}HR;


void* threadFunc(void *threadArgs);
int getRequest(char *requestbuf, struct HttpRequest *httpRequest);
void getCommand(int clientfd, struct HttpRequest *httpRequest);
void responseCode(int clientfd, int code, struct HttpRequest *httpRequest);
int transferfile(int clientfd, FILE *fp, int type, 
                 int rangestart, int totallength);
int sendData(int clientfd, char *buf, int length);


#endif // HTTPSERVER_H_
