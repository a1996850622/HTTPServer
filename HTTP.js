var express = require('express');
var http = require('http');

var app = express();

//app.use('/',express.static(__dirname + '/public'));
app.use('/', express.static(__dirname)); 

var server = http.createServer(app);

console.log("HTTP 1.1 server is up...");

server.listen(8888); //server start 