HTTPServer: httpserver.o ContentType.o
	gcc -g -Wall httpserver.o ContentType.o -o HTTPServer -lpthread

httpserver.o: httpserver.c ContentType.h
	gcc -g -Wall -c httpserver.c

ContentType.o: ContentType.c ContentType.h
	gcc -g -Wall -c ContentType.c 

clean:
	rm -f httpserver.o ContentType.o HTTPServer
