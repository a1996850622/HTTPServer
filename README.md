# HTTPServer

  A simple *C* HTTP 1.0 Server, **for homework** :)
  
  In addition to the "C", I also attach the "Node.js" version.
  
  But the "Node.js" version is HTTP 1.1.
  
  So you can use "wireshark" to compare their packets.


## Install

For `HTTPServer.c`:

- Install
  - `make`
- Clean all build files
  - `make clear`


## Execute

- ##### **`HTTPServer.c`**
  - You need to run it as *root*, it would run the server on port 80 as default:
    - `sudo ./httpserver`
  - Or, Choose a port number, like this:
    - `./httpserver 9487`

- ##### **`HTTP.js`**
  - You need to run it as *root*.
    - `sudo node HTTP.js`


## Test Server

**`wget.sh`** is a shell script, using `wget` command to download the page, 
and save the files to [./wget](./wget/README.md).

you can use this shell script to test the server.

> Change the variable `URL` if you sherver did not use default port 80.

Run it by this command:
- `sh wget.sh`
