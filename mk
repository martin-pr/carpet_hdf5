#!/bin/bash

if [ ! -e bin ]; then
    mkdir bin
fi

cd bin
cmake ..
make -j 12 || exit 1
cd ..
