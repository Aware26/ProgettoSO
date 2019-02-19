/* Grasso Gianluca 046001897 */

/* Client*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <libgen.h>

int sock;
void inviook(int sock);
void *workerthread(void *arg);
void stampafile(int sock);
int main(){
	pthread_t wr;
	char buff[600];
	char utente[20];
    char nomefile[100];
    char conferma[20];
	struct sockaddr_in daddr;
	int len,fi,n = 1;
	FILE *fd = NULL;//file descriptor
	sock=socket(AF_INET,SOCK_STREAM,0);

	daddr.sin_family=AF_INET;
	daddr.sin_port=htons(7777);
	daddr.sin_addr.s_addr=inet_addr("127.0.0.1");

	if(connect(sock,(struct sockaddr*)&daddr,sizeof(daddr))==-1){
		perror("Errore ellòa connect");
		close(sock);
		exit(EXIT_FAILURE);
	}

	//sistema login
	len = read(sock, (void *)buff, 200);
    if(len > 0)
	buff[len] = '\0';
    printf("%s", buff);
    fgets(buff, 200, stdin);
 	write(sock, (void *)buff, strlen(buff));

 	len = read(sock, (void *)buff, 200);
    if(len > 0)
	buff[len] = '\0';
    printf("%s", buff);
    fgets(buff, 200, stdin);
 	write(sock, (void *)buff, strlen(buff));

 	len = read(sock, (void *)buff, 200);
    if(len > 0)
	buff[len] = '\0';
	
	if(strlen(buff) == 6 && strncmp(buff,"errore",6) == 0){
		printf("Errore:Credenziali sbagliate\n");
		close(sock);
		exit(EXIT_FAILURE);
	}

	inviook(sock);

	printf("----Lista dei file-----\n");
	stampafile(sock);

while(1){

    printf("\n");
	printf("PROGETTO SO MENU:\n1)download <pathfile>\n2)upload <filedacaricare>\n3)listhome\n4)esci\nScelta: ");
	sprintf(buff, " ");
	fgets(buff, 499, stdin); 
    write(sock, (void *)buff, strlen(buff));

    if(strlen(buff) == 5 && strncmp(buff, "esci", 4)== 0) {
	    close(sock);
	    exit(EXIT_SUCCESS);
	}

	else if(strlen(buff) > 10 && strncmp(buff, "download ", 9) == 0) {   //nel caso l'utente sceglie download
				len = read(sock, (void *)buff, 199);
		    	if(len > 0)
				     buff[len] = '\0';
		      if(strlen(buff) == 1 && strncmp(buff, "0", 1) == 0) {
				printf("Errore nel download\n");
				 		inviook(sock); //invio conferma al server
			    }
			    else{
			    	
			    	strcpy(nomefile,basename(buff));//estraggo dal pathname assoluto il nome del file tramite questa funzione definita in libgen.h 
			    	printf("%s\n",nomefile);
			    	while(1) {
			    	
				    if((fd = fopen(nomefile,"r")) != NULL) {	    
						printf("\nInserisci un nuovo nome:");
				    	scanf("%s%*c",nomefile);
				    }
				    else
					break;
				}	
		        //creo e apro file in scrittura
				fd = fopen(nomefile, "w");
				
				if(fd == NULL) {
				    printf("\nErrore");
				    close(sock);			
				    exit(EXIT_FAILURE);
				}
				//Qui mando l'ok al server per fargli capire che sono pronto a scaricare il file
				inviook(sock);
			while(1) {
			       len = read(sock, (void *)buff,600);
		    	         if(len > 0)
		                        buff[len] = '\0';
						if(strncmp(buff, "finito", 6) == 0) {
				            inviook(sock);
			                break;
			            }
		           //scrivo nel file i 600 caratteri
				    fprintf(fd,"%s", buff);
				    
				    //mando l'ok al server
				    inviook(sock);

			        }
					
				fclose(fd);
				printf("\n-----File Scaricato correttamente-----\n");		

			    }
	}
	else if(strlen(buff) > 8 && strncmp(buff, "upload ", 7) == 0) {  //nel caso in cui l'utente sceglie upload
						
						nomefile[0]='\0';
						strcpy(nomefile, &buff[7]);//copio il nome del file dopo upload
						
						nomefile[strlen(nomefile)-1] = '\0';//elimino il ritorno a capo
						
						read(sock, (void *)buff,600);//aspetto l ok dal server

						fi=open(nomefile, O_RDONLY);//apro il file in lettura
			        
							if(fi == -1) {//se il file non si apre invio 0
				            sprintf(buff,"0");
				            write(sock, (void *)buff, strlen(buff));
				        	}
			        	else{
			        		sprintf(buff, "1");
	     					write(sock, (void *)buff, strlen(buff));			            
				    	
				    	 read(sock, (void *)buff, 15);//Aspetto ok dal server
				    
			    	   while(1) {
					  
					  	printf("\n---UPLOAD FILE ----\n");  //invio nome del file
					   	printf("Inserisci il nome del file:");
					    scanf("%s%*c", buff);
					    write(sock, (void *)buff, strlen(buff));  
						
			              len = read(sock, (void *)buff,16);
			   
					    if(strncmp(buff,"ok",2) == 0)
								break;
					    else
						printf("Esiste gia' un file con questo nome\n");		
					} 

			 	
				// sono le stesse righe di codice usate nella funzuione download del server ,servono a inviare il file
			 	 while (1) {
			        	n=read(fi,&buff,600);
			       		
			    	if(n>0) {
			        write(sock, (void *)buff, strlen(buff));
			         len = read(sock, (void *)conferma, 16);
			    } 

			    else {
			            sprintf(conferma, "finito");
			             write(sock, (void *)conferma, strlen(conferma));
			            len = read(sock, (void *)conferma, 16);
			        break;
			    }
			  }
			    close(fi);

			    len = read(sock, (void *)buff,100);
			    printf("%s\n",buff);


			    //ricevo e stampo lista aggiornata
			    printf("----Lista dei file-----\n");
				stampafile(sock);
			}
}
			else if(strncmp(buff,"listhome ",9)){
				printf("----Lista dei file-----\n");
				stampafile(sock);

}
}

}


void stampafile(int sock) {
    char buff[500];
    int len;

    while(1) {

	len = read(sock, (void *)buff, 500);
    	if(len > 0)
            buff[len] = '\0';


	if(strncmp(buff, "finito",6) == 0) {
	    inviook(sock);
	    break;
	}
	printf("%s\n", buff);
	inviook(sock);

    }    	
}


void inviook(int sock){
	char buff[16];
	sprintf(buff, "ok");
	write(sock, (void *)buff, strlen(buff)); 
}
