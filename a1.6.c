
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
#include <sys/wait.h> 

#define SIZE    100000

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

    int err, status;
    int p[2];

    //Creating pipe for data transfer from child process to parent process
    if (pipe(p) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    printf("starting---\n");

    //Forking new process
    int f = fork();
    //Mergesort right block on the child process
    if(f == 0) {
        merge_sort(&right_block);
        
        //Return sorted array to parent process through pipe
        close(p[0]);
        write(p[1], right_block.first, right_block.size * sizeof(int));    

    //Mergesort left block on the parent process
    } else if ( f > 0) { 
        merge_sort(&left_block);

        //Reading right array sorted data sent by the child process from the pipe
        close(p[1]);
        read(p[0], right_block.first, right_block.size * sizeof(int));

        //Merging both sides of the array
        merge( &left_block, &right_block);

        printf("---ending\n");
        printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
        exit(EXIT_SUCCESS);
        
    } else {
        perror("Error: Failed to fork thread");
        exit(EXIT_FAILURE);
    }
     
}