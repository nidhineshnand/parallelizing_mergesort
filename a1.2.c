
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
/* Merge sort the data. */
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

    if (argc < 2) {
        size = SIZE;
    } else {
        size = atol(argv[1]);
    }

    //Getting rlimit for memory ans setting new limit
    int val = getrlimit(RLIMIT_AS, &rlim);
    rlim.rlim_cur = size*10;
    if(setrlimit(RLIMIT_STACK, &rlim) != 0){
        perror("WARNING: memory limit couldn't be set:");
    }

    int s;
    pthread_attr_t attr;
    s = pthread_attr_init(&attr);
    if (s != 0){
        handle_error_en(s, "pthread_attr_init");
    }
    //Setting new memory limit on thread
    size_t stack_size;
    pthread_attr_getstacksize(&attr,&stack_size);
    
    stack_size = size*8;
    if( stack_size > PTHREAD_STACK_MIN){
        s = pthread_attr_setstacksize(&attr, stack_size);
    }
    if (s != 0){
        handle_error_en(s, "pthread_attr_setstacksize");
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

    //Creating new thread
    pthread_t thread;
    int err;

    printf("starting---\n");

    //Error checking for thread
    err = pthread_create(&thread, &attr, merge_sort_multi , (void*)&left_block);

    if (err){
        printf("An error occured: %d", err);
        return 1;
    }

    s = pthread_attr_destroy(&attr);
    if (s != 0){
        handle_error_en(s, "pthread_attr_destroy");
    }

    merge_sort(&right_block);
    pthread_join(thread, NULL);
    merge( &left_block, &right_block);

    printf("---ending\n");
    printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
    exit(EXIT_SUCCESS);
}