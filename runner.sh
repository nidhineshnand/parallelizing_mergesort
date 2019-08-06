#!/bin/bash

for i in {1..9} ; do
    file="./a1.$i.c"
    if [ -f $file ]; then

        cc $file -o "a$i.out" -lpthread
        echo "Test Part $i"
        time ./"a$i.out" 10000000
        echo "              "
    fi
done