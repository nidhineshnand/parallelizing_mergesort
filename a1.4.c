
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

#define handle_error_en(en, msg) \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

long number_of_processors;
int number_of_threads;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_attr_t attr;

struct block {
    int size;
    int *first;
};

void *merge_sort_multi(void *args);

// void print_block_data(struct block *blk) {
//     printf("size: %d address: %p\n", blk->size, blk->first);
// }

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
        int created_thread_right, created_thread_left;

        left_block.size = my_data->size / 2;
        left_block.first = my_data->first;
        right_block.size = left_block.size + (my_data->size % 2);
        right_block.first = my_data->first + left_block.size;

        //pthread_mutex_lock(&mut);
        //printf("\nThreads used: %d", number_of_threads);
        //pthread_mutex_unlock(&mut);

        int error = pthread_mutex_lock(&mut);
        if (error == 0 && number_of_threads < number_of_processors){
            number_of_threads++;
            pthread_mutex_unlock(&mut);
            created_thread_left = 1;

            //Creating a thread everytime the merge_sort function is called
            pthread_t thread_left;
            int err = pthread_create(&thread_left, &attr, merge_sort_multi , (void*)&left_block);
            if (err){
                perror("WARNING: thread could not be created:");
            }
            pthread_join(thread_left, NULL);

            pthread_mutex_lock(&mut);
            number_of_threads--;
            pthread_mutex_unlock(&mut);
        } else{
            pthread_mutex_unlock(&mut);
            merge_sort(&left_block);
        }

        pthread_mutex_lock(&mut);
        if (error == 0 && number_of_threads < number_of_processors){
            number_of_threads++;
            pthread_mutex_unlock(&mut);

            //Creating a thread everytime the merge_sort function is called
            pthread_t thread_right;
            int err = pthread_create(&thread_right, &attr, merge_sort_multi , (void*)&right_block);
            if (err){
                perror("WARNING: thread could not be created:");
            }
            pthread_join(thread_right, NULL);

            pthread_mutex_lock(&mut);
            number_of_threads--;
            pthread_mutex_unlock(&mut);
            
        } else{
            pthread_mutex_unlock(&mut);
            merge_sort(&right_block);
        }

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

    //Getting the number of threads online
    number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);
    number_of_threads = 0;

    if (argc < 2) {
        size = SIZE;
    } else {
        size = atol(argv[1]);
    }

    //Getting rlimit for memory and setting new limit
    int val = getrlimit(RLIMIT_AS, &rlim);
    rlim.rlim_cur = size*10;
    if(setrlimit(RLIMIT_STACK, &rlim) != 0){
        perror("WARNING: memory limit couldn't be set:");
    }

    int s;
    
    s = pthread_attr_init(&attr);
    if (s != 0){
        handle_error_en(s, "pthread_attr_init");
    }

    //Setting new memory limit on thread
    size_t stack_size;
    pthread_attr_getstacksize(&attr, &stack_size);
    
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

    printf("starting---\n");

    merge_sort(&start_block);
    printf("---ending\n");
    printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
    exit(EXIT_SUCCESS);
}