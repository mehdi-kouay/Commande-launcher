#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#include <pthread.h>



#define THREADS_NBR 30
typedef struct {
	
    pthread_t threads[THREADS_NBR];
    bool free[THREADS_NBR];
    request_t data[THREADS_NBR];
    sem_t mutex_tab[THREADS_NBR];
    int indexes[THREADS_NBR];
  
} threadgrp_t;

void *run(void * arg);

threadgrp_t *CreateThreads(threadgrp_t *tg) {
	//creation des thread
	int errnum;pthread_t th;
	
	for(int j = 0; j<THREADS_NBR ; j++) {
		
        tg->indexes[j] = j;	    	   
	    if (sem_init(&tg->mutex_tab[j], 1, 0) == -1) {
            perror("sem_init");
            exit(EXIT_FAILURE);
        }

	    if((errnum = pthread_create(&th, NULL,run, &tg->indexes[j])) != 0) {  
	       
	        fprintf(stderr, "pthread_create: %s\n",strerror(errnum));
	    }	
	    
	    tg->threads[j] = th;
	    tg->free[tg->indexes[j]] = true;
    }
    return tg;	
}


int FreeThIndex(threadgrp_t *tg) {
	
    for(int i=0;i<THREADS_NBR;i++ ) {
		
        if(tg->free[i] == true)
        return i;
    }
   
    return -1;
}


void PTH(sem_t *s){
	
    if(sem_wait(s) == -1) {
		
		  perror("SemWait");
		  exit(EXIT_FAILURE);
    }
}

void VTH(sem_t *s) {
	
	if(sem_post(s) == -1) {
		  perror("SemPost");
		  exit(EXIT_FAILURE);
    }
}






