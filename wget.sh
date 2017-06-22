#!/bin/sh

DIR=wget

URL=http://127.0.0.1/test.html

cd $DIR

while true
do
	wget -o - $URL | grep OK
done
