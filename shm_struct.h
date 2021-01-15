#include "queue.h"
#include <semaphore.h>

#define SHM_NAME "/newsharedq4"


typedef struct {
	
    req_queue_t req_queue;
    volatile int client_index;
    sem_t *semIndex;
} shm_struct;


void VSHM(sem_t *s) {
	
    if(sem_post(s) == -1) {
	    perror("SemPost");
	    exit(EXIT_FAILURE);
    }
	
}
		
void PSHM(sem_t *s) {
	
    if(sem_wait(s) == -1) {
        perror("SemWait");
        exit(EXIT_FAILURE);
    }
}	



