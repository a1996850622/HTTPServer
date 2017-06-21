// #include <stdlib.h>
// #include <stdio.h>
#include <string.h>

#include "ContentType.h"

char* ContentType[][2] = {
    {"jpeg", "image/jpeg"              }, 
    {"jpg" , "image/jpeg"              }, 
    {"png" , "image/png"               }, 
    {"gif" , "image/gif"               }, 
    {"txt" , "text/plain"              }, 
    {"htm" , "text/html"               }, 
    {"html", "text/html"               }, 
    {"*"   , "application/octet-stream"}
};

int ContentType_Size = sizeof(ContentType) / sizeof(ContentType[0]);

char* getContentType (char* ExtName){
    int i; // For Ctrl
    for(i = 0;i < ContentType_Size; i++){
        if(strcmp(ContentType[i][0], ExtName) == 0)
            return ContentType[i][1];
    }
    return "application/octet-stream";
}

int getContentTypeSize(){
    return ContentType_Size;
}

