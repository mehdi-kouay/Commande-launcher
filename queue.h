#include <stdio.h>
#include <stdlib.h> 
#include <semaphore.h>
#include "request.h"

#define Q_SIZE 4
#define Q_FULL -1
#define SUCCESS 0

typedef struct {
	
    request_t requests[Q_SIZE];
    int head;
    int tail;
    int size;
    sem_t mutex;
    sem_t vide;
    sem_t plein;	  
} req_queue_t;
	
void V(sem_t *s) {
	
    if(sem_post(s) == -1) {
        perror("Sempost");
        exit(EXIT_FAILURE);
    }
}
		
void P(sem_t *s) {
	
    if(sem_wait(s) == -1) {
        perror("Sempost");
        exit(EXIT_FAILURE);
    }
}	
	
	
void initialize(req_queue_t *q) {
	
	
    q->tail = 0;	 
    q->head = 0;
    q->size = 0;
 
    if (sem_init(&q->mutex ,1, 1) == -1) {
      perror("sem_init");
      exit(EXIT_FAILURE);
    }
 
 
    if (sem_init(&q->vide, 1, Q_SIZE) == -1) {
      perror("sem_init");
      exit(EXIT_FAILURE);
    }
    
    if (sem_init(&q->plein, 1, 0) == -1) {
      perror("sem_init");
      exit(EXIT_FAILURE);
    }

}
	
int enfiler(req_queue_t *q, request_t r) {
	
	/*if(q->size == Q_SIZE)
	  return Q_FULL;*/
	P(&q->vide);
    P(&q->mutex);

	q->requests[q->head] = r;
	q->head = (q->head + 1 ) % Q_SIZE; 
	q->size++;
	
	V(&q->mutex);
	V(&q->plein);
	
	return SUCCESS;
}

request_t defiler(req_queue_t *q) { 
	
	 request_t r;
	 
	P(&q->plein);
	P(&q->mutex);
	
	r = q->requests[q->tail]; 
	q->tail = (q->tail + 1) % Q_SIZE;
	q->size--;
	 
    V(&q->mutex);
	V(&q->vide);
	
	
	return  r;
}




