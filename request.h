#include <string.h>
#include <unistd.h>
#include <sys/types.h>

typedef struct {
	
	//nom du tube du client
    char nom[256];
    //commande envoyé par le client
    char cmd[256];
    //id du processus 
    pid_t processId;
} request_t;

//méthode pour remplire une requette
void set_request(request_t *request,char *nom, char *cmd, pid_t i) {

    strcpy(request->nom, nom);
    strcpy(request->cmd, cmd);
    request->processId = i;
	 
}
