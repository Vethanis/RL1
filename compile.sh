#!/bin/bash

#export CC=/usr/bin/clang
#export CXX=/usr/bin/clang++

if [ "$1" == "clean" ]; then
    rm -rf build
fi

if [ ! -d "./build" ]; then
    mkdir build
    cd build
    cmake .. -G "Unix Makefiles"
    cd ../
fi

TYPE="Release"

if [ "$1" == "debug" ] || [ "$2" == "debug" ] ; then
    TYPE="Debug"
fi

cd build 
make -j16
cd ..

if [[ $? > 0 ]]; then
    exit 1
fi

mkdir -p bin/

cp -r assets bin/

if [ "$1" == "debug" ] || [ "$2" == "debug" ] ; then
    echo ""
else
    cd bin
    ./rl1.exe
    cd ..
fi
