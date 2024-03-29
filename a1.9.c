
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
#include <sys/mman.h>
#include <sys/wait.h> 

#define SIZE    1000

long number_of_processors;
static int *number_of_processes;
int status = 0;
int parent;
static int *data;
pthread_mutex_t *mut;

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
    
    if (my_data->size > 1) {
        struct block left_block;
        struct block right_block;
        left_block.size = my_data->size / 2;
        left_block.first = my_data->first;
        right_block.size = left_block.size + (my_data->size % 2);
        right_block.first = my_data->first + left_block.size; 

        //Locking number_of_processes and checking to see if a new process can be created
        pthread_mutex_lock(mut);
        if (*number_of_processes < number_of_processors){    
            *number_of_processes = *number_of_processes + 1;
            pthread_mutex_unlock(mut);
            
            //Forking new process
            int f = fork();
            //Mergesort right block on the child process
            if(f == 0) {
                merge_sort(&right_block); 
                
                //Decrementing number_of_processes counter as the child process has finished
                pthread_mutex_lock(mut);
                *number_of_processes = *number_of_processes - 1;
                pthread_mutex_unlock(mut);
                
                exit(EXIT_SUCCESS);   
                
            //Mergesort left block on the parent process
            } else if ( f > 0) {
                merge_sort(&left_block);
                waitpid(f, NULL, 0); //Waiting for child process to completely finish before merging the array
                merge( &left_block, &right_block);

            } else {
                perror("Error: Failed to fork thread");
                exit(EXIT_FAILURE);
            }
            
        } else {
                pthread_mutex_unlock(mut);
                 //If number_of_processes > number of processors available, mergesort is run on the same process as parent
                merge_sort(&right_block);
                merge_sort(&left_block);
                merge( &left_block, &right_block);
                
        }
        
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

    //Getting the number of processors online
    number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);
    //Creating shared memory for number_of_processes variable 
    number_of_processes = (int*)mmap(NULL, sizeof *number_of_processes, PROT_READ | PROT_WRITE, 
                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *number_of_processes = 0;

    if (argc < 2) {
        size = SIZE;
    } else {
        size = atol(argv[1]);
    }

   //Getting rlimit for memory and setting new limitit
    int val = getrlimit(RLIMIT_STACK, &rlim);
    rlim.rlim_cur = size*12;
    if(setrlimit(RLIMIT_STACK, &rlim) != 0){
        perror("WARNING: memory limit couldn't be set");
        exit(EXIT_FAILURE);
    }

    //Creating shared memory for the data array and mutex
    struct block start_block;
    data =  mmap(NULL, sizeof(int) * size, PROT_READ | PROT_WRITE, 
                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    mut =  mmap(NULL, sizeof(*mut), PROT_READ | PROT_WRITE, 
               MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    //Initializing attribute for mutex
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mut, &attr);

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