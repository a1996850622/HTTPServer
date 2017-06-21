# HTTPServer


### Introduce
  A simple *C* HTTP 1.0 Server, for homework :)


### The File That I Attached
  In addition to the "C Program", I also attach the "Node.js" version.
  But the "Node.js" version is HTTP 1.1.
  So you can use "wireshark" to compare their packets.


### Install
- ##### Install
  - `make`
- ##### Clean all build files
  - `make clear`


### Execute
- ##### If you want execute **`HTTPServer.c`**
  You can enter the following commands on your terminal:
  - `sudo ./httpserver`
  It would run the server on port 80 as default.
  Or, Choose a port number:
  - `./httpserver 9487`

- ##### If you want to execute **`HTTP.js`**
  You can enter the following command:
  1. `sudo node HTTP.js`


### Test Server
  You can see the "wget.sh" file.
  After running the httpserver.c, you can also use "sh wget.sh" command to test the server.
