
/*
    The Merge Sort to use for Operating Systems Assignment 1 2019
    written by Robert Sheehan

    Modified by: Nidhinesh Nand
    UPI: nnan773

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>
#include <inttypes.h>

#define SIZE    100000000
#define PTHREAD_STACK_MIN 16384

struct block {
    int size;
    int *first;
};

/* Combine the two halves back together. */
void merge(struct block *left, struct block *right) {
    int combined[left->size + right->size];
    int dest = 0, l = 0, r = 0;
    while (l < left->size && r < right->size) {
        if (left->first[l] < right->first[r])
            combined[dest++] = left->first[l++];
        else
            combined[dest++] = right->first[r++];
    }
    while (l < left->size)
        combined[dest++] = left->first[l++];
    while (r < right->size)
        combined[dest++] = right->first[r++];
    memmove(left->first, combined, (left->size + right->size) * sizeof(int));
}

/* Merge sort the data. */
void merge_sort(struct block *my_data) {
    // print_block_data(my_data);
    if (my_data->size > 1) {
        struct block left_block;
        struct block right_block;
        left_block.size = my_data->size / 2;
        left_block.first = my_data->first;
        right_block.size = left_block.size + (my_data->size % 2);
        right_block.first = my_data->first + left_block.size;
        merge_sort(&left_block);
        merge_sort(&right_block);
        merge(&left_block, &right_block);
    }
}
/* Adapter method that allows merge sort to be directly called by thread without changing anything with mergesort */
void *merge_sort_multi(void *args) {
    struct block *my_data = args;
    merge_sort(my_data);
}


/* Check to see if the data is sorted. */
bool is_sorted(int data[], int size) {
    bool sorted = true;
    for (int i = 0; i < size - 1; i++) {
        if (data[i] > data[i + 1])
            sorted = false;
    }
    return sorted;
}

int main(int argc, char *argv[]) {
    long size;
    struct rlimit rlim;
    int err;

    if (argc < 2) {
        size = SIZE;
    } else {
        size = atol(argv[1]);
    }

    //Getting rlimit for stack and setting new limit
    int val = getrlimit(RLIMIT_AS, &rlim);
    rlim.rlim_cur = size*12;

    if(setrlimit(RLIMIT_STACK, &rlim) != 0){
        perror("Error: Memory limit couldn't be set:");
        exit(EXIT_FAILURE);
    }

    //Initilizing attribute for thread
    pthread_attr_t attr;
    err = pthread_attr_init(&attr);
    if (err != 0){
        perror("Error: Thread attribute not initilized");
        exit(EXIT_FAILURE);
    }

    //Setting new memory limit on thread
    size_t stack_size;
    pthread_attr_getstacksize(&attr, &stack_size);
    
    //Setting stack size only if the size varible is greate then the lower limit of stack size
    stack_size = size*10;
    if( stack_size > PTHREAD_STACK_MIN){
        err = pthread_attr_setstacksize(&attr, stack_size);
    }
    if (err != 0){
        perror("Error: Thread stack size was not be changed");
        exit(EXIT_FAILURE);
    }

    struct block start_block;
    int data[size];
    start_block.size = size;
    start_block.first = data;
    for (int i = 0; i < size; i++) {
        data[i] = rand();
    }

    //Setting the different halves of the block
    struct block left_block;
    struct block right_block;
    left_block.size = start_block.size / 2;
    left_block.first = start_block.first;
    right_block.size = left_block.size + (start_block.size % 2);
    right_block.first = start_block.first + left_block.size;


    pthread_t thread;
    printf("starting---\n");

     //Creating new thread with set attributes and callng merge sort
    err = pthread_create(&thread, &attr, merge_sort_multi , (void*)&left_block);
    if (err){
        perror("Error: Thread could not be created");
        exit(EXIT_FAILURE);
    }

    err = pthread_attr_destroy(&attr);
    if (err != 0){
        perror("Warning: Thread attribute could not be destroyed");
    }

    merge_sort(&right_block);

    //Joining thread before merging
    pthread_join(thread, NULL);
    merge( &left_block, &right_block);

    printf("---ending\n");
    printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
    exit(EXIT_SUCCESS);
}