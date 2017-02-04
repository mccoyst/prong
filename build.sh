#!/bin/sh
os=$(uname)
if [ $os = Linux ]; then
	exec ninja -f build.linux
else
	exec ninja -f build.osx
fi
