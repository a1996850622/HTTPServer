# HTTPServer


### Introduce
  A simple *C* HTTP 1.0 Server, for homework :)


### The File That I Attached
  In addition to the "C Program", I also attach the "Node.js" version.
  But the "Node.js" version is HTTP 1.1.
  So you can use "wireshark" to compare their packets.


### Install
- ##### If you want execute "httpserver.c"
  You can enter the following commands on your terminal:
  1. `gcc -g -Wall httpserver.c -o httpserver -lpthread`
  1. `sudo ./httpserver`

- ##### If you want to execute "HTTP.js"
  You can enter the following command:
  1. `sudo node HTTP.js`


### Test Shell
  You can see the "wget.sh" file.
  After running the httpserver.c, you can also use "sh wget.sh" command to test the server.
