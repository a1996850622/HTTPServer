HTTPServer: HTTPServer.o ContentType.o
	gcc -g -Wall HTTPServer.o ContentType.o -o HTTPServer -lpthread

HTTPServer.o: HTTPServer.c ContentType.h
	gcc -g -Wall -c HTTPServer.c

ContentType.o: ContentType.c ContentType.h
	gcc -g -Wall -c ContentType.c 

clean:
	rm -f HTTPServer.o ContentType.o HTTPServer
