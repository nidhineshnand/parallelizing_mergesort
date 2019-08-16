

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

long number_of_processors;
int number_of_threads;
long size;

pthread_spinlock_t spinlock; 

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
void *merge_sort(void *args) {
    struct block *my_data = args;

    int err;
    pthread_attr_t attr;

    //Initilizing thread attribute
    err = pthread_attr_init(&attr);
    if (err != 0){
        perror("Error: Thread attribute not initilized");
        exit(EXIT_FAILURE);
    }

  //Setting stack size only if the size varible is greater then the lower limit of stack size
    size_t stack_size = size*10;
    if( stack_size > PTHREAD_STACK_MIN){
        err = pthread_attr_setstacksize(&attr, stack_size);
    }
    if (err != 0){
        perror("Error: Thread stack size was not be changed");
        exit(EXIT_FAILURE);
    }
    
    if (my_data->size > 1) {
        struct block left_block;
        struct block right_block;
        left_block.size = my_data->size / 2;
        left_block.first = my_data->first;
        right_block.size = left_block.size + (my_data->size % 2);
        right_block.first = my_data->first + left_block.size; 
    
        pthread_t thread_left;
        int err, s, thread_left_created;
        thread_left_created = 0;

        //Locking variable number_of_threads and creating thread if the variable is less them the number of cores in the system
        int error = pthread_spin_lock(&spinlock);
        if (error == 0 && number_of_threads < number_of_processors){
            number_of_threads++;
            pthread_spin_unlock(&spinlock);
            thread_left_created = 1;

            //Creating a thread everytime the merge_sort function is called
            err = pthread_create(&thread_left, &attr, merge_sort , (void*)&left_block);
            if (err){
                perror("WARNING: thread could not be created:");
                exit(EXIT_FAILURE);
            }

        } else{
            //If number_of_threads > number of processors available, mergesort is run on the same thread as parent
            pthread_spin_unlock(&spinlock); //spin lock at top of if statement freed here
            merge_sort(&left_block);
        }

        //Running mergesort on the right block on the parent thread
        merge_sort(&right_block);

        if(thread_left_created){
            pthread_join(thread_left, NULL);

            //Decrementing counter for threads
            pthread_spin_lock(&spinlock);
            number_of_threads--;
            pthread_spin_unlock(&spinlock);
        }
        
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
    struct rlimit rlim;
    
    //Initilizing spin locks
    pthread_spin_init(&spinlock, 0);

    //Getting the number of threads online
    number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);
    number_of_threads = 0;

    if (argc < 2) {
        size = SIZE;
    } else {
        size = atol(argv[1]);
    }

    //Getting rlimit for memory and setting new limit
    int val = getrlimit(RLIMIT_STACK, &rlim);
    rlim.rlim_cur = size*12;
    if(setrlimit(RLIMIT_STACK, &rlim) != 0){
        perror("WARNING: memory limit couldn't be set");
        exit(EXIT_FAILURE);
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