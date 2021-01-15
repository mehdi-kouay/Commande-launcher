#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <signal.h>

#include "shm_struct.h"


#define PIPE_NAME "CLIENT"

void my_handler(int signum);

    // Tableau de charactére qui contiendra le nom du tube 
    char pipeName[9];

int main(int argc, char **argv) {
   	
   	
   	// descripteur de memoire partagé
    int shm_fd;
    // Pointeur sur mémoire partagé
    shm_struct *shm = malloc(sizeof(shm_struct));
    // Pointeur sur la file de la mémoire partagé
    req_queue_t *requestQ ;
    //variable contenant la requéte à enfiler
    request_t requete;
    //descripteur de tube ouver en lecture
    int out;
    //tableau de char pour stocker le résultat envoyer par le démon
    char buffer[1024] = {0};
    //variable pour stocker l'id du processus courant
    pid_t processId;
    
    
    
    
    
    
    
    //ouverture de la mémoire partagé
   	if((shm_fd = shm_open(SHM_NAME, O_RDWR ,0666) )== -1 ) {
	   	perror("shm_open");
        exit(EXIT_FAILURE);
	}
    //projection de la mémoire partagé sur l'espace d'adressage
    if((shm= (shm_struct *) mmap(NULL, sizeof(shm_struct), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == MAP_FAILED) {
	    perror("mmap");
        exit(EXIT_FAILURE);
	}
    //pointer sur la file de la mémoire partagé
    requestQ = &shm->req_queue;

    
    
    
    
    //initialisation du semaphore sur l'indexe du client de shm
    shm->semIndex = malloc(sizeof(sem_t));
    if (sem_init(shm->semIndex, 1, 1) == -1) {
      perror("sem_init");
      exit(EXIT_FAILURE);
    }

    
    PSHM(shm->semIndex);
    
    //Incrémentation de l'indexe de clients
    shm->client_index++;
    //Construction du nom de tube 
    snprintf(pipeName, 9, "%s%d",PIPE_NAME, shm->client_index);
    
    VSHM(shm->semIndex);
    
    
    //Creation du tube nommé 
    if (mkfifo(pipeName, S_IRUSR | S_IWUSR) == -1) {
		
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    

        
    
    
  
    
 while(1) {
	 

    printf("\ncmd:");
    char cmd[256];
    //Get client input
    fgets(cmd,256,stdin);
    strtok(cmd, "\n");
    //récupérer l'id du processus courant
    processId = getpid();
    //remplire la requette à enfiler
    set_request(&requete, pipeName, cmd,processId); 
    //enfiler la requette
    enfiler(requestQ, requete);
    //manupulation de signal
    signal(SIGUSR1,my_handler);
    signal(SIGUSR2,my_handler);
 


    //Ouverture de tube en lecture
	out = open(requete.nom, O_RDONLY);
    if(out == -1) {
       perror("open");
	   exit(EXIT_FAILURE);
    }
    
	//lecture des données envoyées par le serveur		
	while (read(out, buffer, sizeof(buffer)) != 0) {
		
         write(1, buffer, strlen(buffer));
         memset (buffer, 0, sizeof(buffer));
      }
    
    //Fermeture de tube 
    if(close(out) == -1) {
		
      perror("close");
	  exit(EXIT_FAILURE);
   }
   
  


  }

  //supression de tube nommé
  if(unlink(requete.nom) == -1) {
      perror("unlink");
	  exit(EXIT_FAILURE);
   }
  //fermeture de la mémoire partagé 
  if (close(shm_fd) == -1) {
    perror("close");
    exit(EXIT_FAILURE);
  }
  
}


//fonction à executer lors de reception d'un signal SIGUSR1 ou SIGUSR2
void my_handler(int signum) {
	
    if (signum == SIGUSR1) {
        write(1, "Pas de thread disponible\n", 32);
    } else if (signum == SIGUSR2){
		
        write(1, "serveur éteint \n", 19);
        if(unlink(pipeName) == -1) {
            perror("unlink");
            exit(EXIT_FAILURE);
        }
        exit(0);
    }
}







