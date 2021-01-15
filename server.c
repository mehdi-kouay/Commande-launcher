#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "shm_struct.h"
#include "threadgrp_t.h"




//fonction pour préparer les arguments pour execvp
void parseCmd(char* str, char** parsed) ;
//fonction pour executer une commande
void execCmd(char** parsed, char *name) ;

bool simpleCMD(char **parsed, int *cmd_index);




//Structure contenant l'ensemble de thread
threadgrp_t *thread_grp;
//Pointeur sur la structure contenant la mémoire partagée
shm_struct *shm;


int main(int argc, char **argv) {
	
	//entier pour stocker l'id d'un thread libre
    int i;
	//descripteur du fichier contenant la mémoire partagée
	int shm_fd;
	//Pointeur 
	req_queue_t *requestQ;
	//requéte 
	request_t req ;
	
	
	//allocation mémoire pour le pointeur du groupe de thread	
	thread_grp = malloc(sizeof(threadgrp_t)*THREADS_NBR);
	
   	//Création dela mémoire partagé  	
   	
   	if((shm_fd = shm_open(SHM_NAME, O_RDWR |O_CREAT,0666) )== -1 ) { 
	
        perror("shm_open");
        exit(EXIT_FAILURE);
	}
    if (ftruncate(shm_fd, sizeof(shm_struct)*30) == -1) {
		
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
    if((shm = (shm_struct *) mmap(NULL, sizeof(shm_struct), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == MAP_FAILED) {
	    perror("mmap");
        exit(EXIT_FAILURE);
	}
	
	//creation des threads
	thread_grp = CreateThreads(thread_grp) ;
	//pointer sur la file de la mémoire partagée
    requestQ = &shm->req_queue;
    
    //initialisation de la file
    initialize(requestQ);
    
    
    //boucle principale du sérveur
    while(1) {
        
        //récupération de la premiere commande arrivée
        req = defiler(requestQ);
	  	//récupérer l'index d'un thread libre
	    i = FreeThIndex(thread_grp);
	    //envoyer un signal SIGUSR1 au client s'il n'y a pas de thread libre
	    if(i==-1) {

            if (kill(req.processId,SIGUSR1) != 0) {
			  
                printf("\nEnvoi de signal échoué!\n");
            }
		} 
	    //attribution de la requétte récupérée au thread libre 
	    thread_grp->data[i] = req;
	    //changer l'état du thread dans la structure thread_group
	    thread_grp->free[i] = false; 
	    
	    VTH(&thread_grp->mutex_tab[i]);	
    }
}	



// Function where the system command is executed 
void ExecuteCmd(char** parsed, char *name) { 
	
    // Forking a child 
    
    
    
	
    pid_t pid = fork();   
    if (pid == -1) { 
		
        printf("\nFailed forking child.."); 
        return; 
    } else if (pid == 0) {
		
		 int in = open(name, O_RDWR, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
         if(in==-1) {
		
		   perror("open User pipe");
		   exit(EXIT_FAILURE);
	     } 
	     
	     dup2(in,STDOUT_FILENO);
	
	     if(close(in) == -1) {
           perror("close");
	       exit(EXIT_FAILURE);
         }
		 
         
         if (execvp(parsed[0], parsed) == -1) { 
           printf("Commande '%s' non trouvé",parsed[0]); 	
         } 
         //dup2(STDOUT_FILENO,STDOUT_FILENO);
         
         exit(0);
	     
	      
    } else { 
        // waiting for child to terminate 
        wait(NULL);  
        return; 
    } 
} 


  
// function for parsing command words 
void parseCmd(char* str, char** parsed) {
	 
    int i; 
    for (i = 0; i < 256; i++) { 
		
        parsed[i] = strsep(&str, " ");
        if (parsed[i] == NULL) 
            break; 
        if (strlen(parsed[i]) == 0) 
            i--; 
    } 
} 
  




void *run(void * arg) {
	
   int x = *(int *)arg;
   int *cmd_index = (int*)malloc(sizeof(int));
   

   while(true) {
      PTH(&thread_grp->mutex_tab[x]);
      
	  
	  char *parsed_cmd[256];
      request_t req = thread_grp->data[x];
      // execution en cas d'une commande sans pipe   
      parseCmd(req.cmd, parsed_cmd);
      
    
   if(simpleCMD(parsed_cmd,cmd_index)) {
		
      switch (*cmd_index) {
		  
         case 0:
				kill(req.processId,SIGKILL);
				if(unlink(req.nom) == -1) {
				   perror("unlink");
				   exit(EXIT_FAILURE);
				}
                break;
                
	     case 1:
	            kill(req.processId,SIGUSR2);
				printf("Server got turned off\n");
				if (shm_unlink(SHM_NAME) == -1) {
		
                   perror("shm_unlink");
                   exit(EXIT_FAILURE);
                }
				exit(0);
				
				
	     default:break;
	   }
	   
   }else {
      ExecuteCmd(parsed_cmd,req.nom); 
   }
   
   thread_grp->free[x] = true;
   
   }
        
   pthread_exit(NULL);
}	
  
bool simpleCMD(char **parsed, int *cmd_index) {
   bool is_simple = false;
   char * simple_cmds[2] = {"exit","turnoff"};
   
   
   
   for (int c = 0; c < 2; c++) { 
        if (strcmp(parsed[0], simple_cmds[c]) == 0) { 
			
            *cmd_index = c ;
            is_simple = true; 
            break; 
        } 
    } 
    
return is_simple;
}  


