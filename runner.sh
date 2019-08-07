#!/bin/bash


#Compiling
cc $file "a1.1.c" -o a1.out -lpthread
cc $file "a1.2.c" -o a2.out -lpthread
cc $file "a1.3.c" -o a3.out -lpthread
cc $file "a1.4.c" -o a4.out -lpthread
cc $file "a1.5.c" -o a5.out -lpthread
cc $file "a1.6.c" -o a6.out -lpthread
cc $file "a1.7.c" -o a7.out -lpthread
cc $file "a1.8.c" -o a8.out -lpthread
cc $file "a1.9.c" -o a9.out -lpthread
cc $file "a1.10.c" -o a10.out -lpthread -lm

for i in {1..10}
    echo "Test Part $i"
    grep cpu | time ./"a$i.out" 10000000 
    echo "              "
done
