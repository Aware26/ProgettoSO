#define N 200
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
//lista dei file
typedef struct nodolista {
    char *pathfile;
    struct nodolista *next;
    int isdirectory;
} nodo_lista;

typedef nodo_lista* listafile; 

//dichiarazione funzioni
void InserisciinLista(listafile *lis,char *filename,int f);
void stampalista();
void CancellaElemento(char *elem);
void setNullList(listafile *lis);
void ls_directory(listafile *lis, char *arg);
void *thread_function(void *arg);
void *workerthread(void *cs);
void concatlis(listafile lis, listafile *main_list);
listafile trovadir();
void invialista(listafile lis,int sock, char *path);

void scanhome(char *path);
void download(char *file, int sock);
void upload(int sock);

void inviook(int sock);

//dichiarazione lista globale
listafile list;

pthread_mutex_t mutexlista; // mutex che protegge lista globale 
pthread_mutex_t mutexuscita;

int *controllouscita; //variabile di controllo per la terminazione dei thread

//Credenziali Utente
char utente[]="admin";
char password[]="admin";
char pass[40];
char user[80];
char nomeuser[20];



int main(void) {
  int i=0;

    int serversock; 
    int clientsock; 
    struct sockaddr_in baddr;
    pthread_t worker;

    serversock=socket(AF_INET,SOCK_STREAM,0);
    if(serversock == -1) {
    perror("Errore connessione\n");
    exit(EXIT_FAILURE);
    }

    baddr.sin_family=AF_INET;
    baddr.sin_port=htons(7777);
    baddr.sin_addr.s_addr=htonl(INADDR_ANY);

    if(bind(serversock,(struct sockaddr*)&baddr,sizeof(baddr))==-1){
        perror("Errore nella Bind\n");
       exit(EXIT_FAILURE);
    }
    if(listen(serversock, 20) == -1) {
    perror("Errore Listen\n");
    exit(EXIT_FAILURE);
    }
    
    scanhome("/home/");//listing della home

    printf("\n----Pronto a ricevere connessioni----\n");
while(1){

    clientsock=accept(serversock,NULL,NULL);
    if(clientsock == -1) {
    perror("Errore Listen\n");
    exit(EXIT_FAILURE);
    }
    printf("Nuova Connessione\n");

    if(pthread_create(&worker,NULL,workerthread, (void *)&clientsock) != 0) { 
        perror("Thread creation failed\n");
        close(clientsock); 
    }

}
}
 
void *workerthread(void *cs){
        int sock;
        char buff[1000],buff2[200];

        
        size_t len;
        
        sock = (*(int *)cs); 
       //Sistema login minimale
       strcpy(user, "Utente e password standard:admin admin\nInserisci username:");
        write(sock, (void *)user, strlen(user));
        len = read(sock, (void *)user, 79);
        if(len > 0)
         user[len-1]='\0';

        strcpy(pass, "Inserisci password:");
        write(sock, (void *)pass, strlen(pass));
        len = read(sock, (void *)pass, 39);
        if(len > 0)
        pass[len-1]='\0';

        if(strcmp(pass,password) != 0 || strcmp(user,utente)!= 0){
            strcpy(buff,"errore");
             write(sock, (void *)buff, strlen(buff));
             printf("Client disconnesso(Credenziali non corrette)\n");
             close(sock);
             pthread_exit(NULL);
         }
            strcpy(buff,"welcome");
            write(sock, (void *)buff, strlen(buff));
           
            len = read(sock, (void *)buff, 99);
            if(len > 0)
            buff[len]='\0';
                if(len == 4 && strncmp(buff, "ok", 2) != 0) {
                printf("Client disconnesso\n");
                close(sock);
                pthread_exit(NULL);
                 }
//invia la lista
            pthread_mutex_lock(&mutexlista);
            invialista(list, sock,"/home");
            pthread_mutex_unlock(&mutexlista);
 while(1) {

        len = read(sock, (void *)buff, 999);
    if(len > 0)
        buff[len-1]='\0';
    if(strlen(buff) > 10 && strncmp(buff, "download ", 9) == 0) {       
            strcpy(buff, &buff[9]); 
            download(buff, sock); 
    }
    else if(len > 8 && strncmp(buff, "upload ",7) == 0) {      
            upload(sock); 
    }
    else if(len>=9 && strncmp(buff,"listhome ",9)){
                pthread_mutex_lock(&mutexlista);
                invialista(list, sock,"/home");
                pthread_mutex_unlock(&mutexlista);
         }
    else if(len==5 && strncmp(buff, "esci", 4) == 0) 
        break; 
    else {
        sprintf(buff, "Comando non esistente o non valido\n");
        write(sock, (void *)buff, strlen(buff));
    }

    }      

    printf("Client disconnesso\n");
    close(sock);




}
void scanhome(char *path){
    int i;
    //Ho trovato su internet un sistema migliore accedere al numero di core tramite la funzione get_nprocs() definita in sys/sysinfo.h>,questa funzione ci ritorna il numero di core
    const int corenumber = get_nprocs();
    controllouscita=(int *)malloc(corenumber*sizeof(int)); 
    pthread_t idthread[corenumber];
    //inizializzazione mutex
    pthread_mutex_init(&mutexlista, NULL);
    pthread_mutex_init(&mutexuscita, NULL);

    printf("\n---- Il numero di core e' %d ----\n", corenumber);

    setNullList(&list);
     ls_directory(&list, path); // lista la directory passata come argomento

    for(i=0; i<corenumber; i++)
        controllouscita[i]=0;

    //Creo la pool di thread: Un thread per ogni core
    for(i=0; i<corenumber; i++) {
	if(pthread_create(&idthread[i], NULL, thread_function, (void *)(intptr_t)i) != 0) { 
	    printf("CREAZIONE THREAD FALLITA\n");
	    exit(EXIT_FAILURE); 
	}	
    }
    printf("\nRicerca file in corso.......\n");
	fflush(stdout);
    //Prima di uscire il main thread aspetta che finiscono gli altri thread
    for(i=0; i<corenumber; i++){
	if(pthread_join(idthread[i],NULL) != 0) {
	    printf("Join Fallita\n");
	    exit(EXIT_FAILURE);
	}
    }
    printf("\nRicerca finita\n");
    pthread_mutex_destroy(&mutexlista);
    pthread_mutex_destroy(&mutexuscita);
    }

void *thread_function(void *arg) {
    
    const int corenumber = get_nprocs();
    int time_to_exit=0; 
    int somma=0;
    listafile dir; 
    listafile temp;   

    while(!time_to_exit) {
                pthread_mutex_lock(&mutexlista);
                dir=trovadir(); 

	if(dir!=NULL) {
        CancellaElemento(dir->pathfile); //elimina la directory
	    pthread_mutex_unlock(&mutexlista);
	    pthread_mutex_lock(&mutexuscita);
	    controllouscita[(intptr_t)arg]=0;
        pthread_mutex_unlock(&mutexuscita);
        temp=NULL;
	    ls_directory(&temp, dir->pathfile);
	    free(dir);
        pthread_mutex_lock(&mutexlista);
        concatlis(temp, &list);
        pthread_mutex_unlock(&mutexlista);

	  } else {
         pthread_mutex_lock(&mutexuscita); 
	    controllouscita[(intptr_t)arg]=1;
	     somma=0;

	   //controlla se gli altri thread hanno finito
	    for(int i=0; i<corenumber; i++)
	        if(controllouscita[i] == 1) 
		    somma++;

	    if(somma == corenumber) 
	           	time_to_exit=1;
	    else 
		       time_to_exit=0; 
        pthread_mutex_unlock(&mutexuscita);
	    pthread_mutex_unlock(&mutexlista);
      } 
     }
    pthread_exit(NULL);
}
void setNullList(listafile *lis) {
    *lis = NULL;
}

void InserisciinLista(listafile *lis,char *filename,int f) {
/* Inserisce l'elemento elem in testa alla lista lis. */

    listafile paux;

    if ((paux = (listafile)malloc(sizeof(nodo_lista))) == NULL) {
        printf("Errore: impossibile allocare memoria");
        exit(1);
    }                                     
    paux->pathfile=filename;
    paux->isdirectory=f;                     
    paux->next = *lis;                      
    *lis = paux;                            
} 

void stampalista() {
printf("\nSTAMPO LISTA\n");
listafile plis=list;
   while (plis) {
        printf("File:%s\n",plis->pathfile);
        plis = plis->next;
    }
    printf("\n");
}

//Funzione che concatena due liste
void concatlis(listafile lis, listafile *lis2) {

    if((*lis2) == NULL) {
	*lis2=lis;
	       return;
     }

    if((*lis2)->next == NULL) {
	       (*lis2)->next=lis;
	       return;
    } 
    else
	concatlis(lis,&(*lis2)->next);

}

listafile trovadir() {

    listafile lis=list;
    
    while (lis != NULL) {
	           if(lis->isdirectory==1) 	
		              return lis;
                         lis=lis->next;
    }
    return NULL;  
  
}

void CancellaElemento(char *ele) {

    listafile prec;    /* puntatore all'elemento precedente */
    listafile corr;    /* puntatore all'elemento corrente */
    int trovato;      /* usato per terminare la scansione */

    if (list!= NULL)
        if (!strcmp((list)->pathfile,ele)) { /* cancella il primo elemento */
            prec = list;
            list = (list)->next;
            //free(prec);
              }
        else {         /* scansione della lista e cancellazione dell'elemento */
            prec = list;
            corr = prec->next;                      /* 1 */
            trovato = 0;
            while (corr != NULL && !trovato)
                if (!strcmp(corr->pathfile, ele)) {       /* cancella l'elemento */
                    trovato = 1;                        /* forza l'uscita dal ciclo */
                    prec->next = corr->next;            /* 2 */
                  //  free(corr);                         /* 3 */
                }
                else {                            
                    prec = prec->next;
                    corr = prec->next;                  
                }
        }
}


//funzione che inserisce i file trovati in una lista
void ls_directory(listafile *lis, char *t) {
    
    char *dirname;
    struct dirent *de; 

    DIR *dir; 
    if((dir = opendir(t)) == NULL) {
     printf("ERRORE -- La directory non si puo' aprire %s\n",t);
       exit(EXIT_FAILURE);
     }

    while ((de = readdir(dir)) != NULL) {
         //non aggiungo la directory corrente e la directory superiore
       if(strcmp(de -> d_name, ".") == 0 || strcmp(de-> d_name, "..") == 0) {
            continue;
        } else {

        if((dirname = (char *) malloc((strlen(t)+strlen(de->d_name)+2)*sizeof(char))) == NULL)
        {
        printf("Errore allocazione memoria non riuscita");
        exit(EXIT_FAILURE);
      }
        strcpy(dirname,"");
	    strcat(dirname, t);


	if(dirname[strlen(dirname) -1] != '/')
    	strcat(dirname, "/");
	    strcat(dirname, de->d_name);
		//Uso il terzo parametro della funzione per indicare se l elemento inserito in lista e' una directory o no
	        if(de->d_type == DT_DIR)
	            InserisciinLista(lis, dirname, 1); 
	        else 
	            InserisciinLista(lis, dirname, 0);

 	    dirname=NULL;
	    free(dirname);
         }
     }
     closedir(dir);
    
}

//Funzioni Socket

void invialista(listafile lis,int sock, char *path){

    int count = 0;
    int i = 0;
    char conferma[16];
    int len = 0;
    char *temp;

    while(lis != NULL) {
      
                temp=lis->pathfile;
                write(sock, (void *)temp, strlen(temp));
                len = read(sock, (void *)conferma,15);
        
            lis=lis->next;
    }
        sprintf(conferma, "finito");
        write(sock, (void *)conferma, strlen(conferma));
        len = read(sock, (void *)conferma,16);

}
void upload(int sock){
       
        char buff[600]; 
        int len=0;
        
        FILE *fd = NULL;

        char nomefile[200]="\0";

        //invio okay al client 
        inviook(sock);
    
        len = read(sock, (void *)buff, 199);
        if(len > 0)
             buff[len] = '\0';

        if(strncmp(buff,"0",1) == 0) {
                printf("Errore nel apertura file\n");
               return;
        }

        inviook(sock);
        
        while(1) {
 
        len = read(sock, (void *)buff,600);
            if(len > 0)
            buff[len]='\0';
	getlogin_r(nomeuser,20);
        printf("----Carico %s della directory /home/\n",buff);
        
        nomefile[0] = '\0';

        strcat(nomefile,"/home/"); //concateno /home/ ovvero la directory dove verrà salvato il file
        strcat(nomefile,nomeuser); 
	strcat(nomefile,"/");       
	strcat(nomefile,buff);

      fd = fopen(nomefile,"r");
     if(fd != NULL) {     //Se il file già esiste invio un messaggio di errore altrimenti ok 
            
            sprintf(buff,"rrore");
            write(sock,(void *)buff, strlen(buff));
            fclose(fd);   
            }
        else {
        inviook(sock);
        break;
    }  
        
    }

      //creo il file in scrittura

 if((fd = fopen(nomefile, "w"))== NULL) {
        printf("\nErrore file");
         close(sock);
        pthread_exit(NULL);         
    }
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
             //mando l'ok al client
            inviook(sock);

            }
            
    
    //aggiorno e invio la nuova lista dei file
    pthread_mutex_lock(&mutexlista);
    printf("Aggionamento lista causa upload\n");
    scanhome("/home/");
    invialista(list, sock,"/home");
    pthread_mutex_unlock(&mutexlista); 

     fclose(fd); 
}

//funzione che permette di inviare un file al client 
void download(char *file, int sock) {
    
    char conferma[16];
    char buff[600]; 
    int n = 1,fd,len =0;

    fd=open(file, O_RDONLY);//apro il file in lettura
        if(fd == -1) {//se il file non si apre invio 0
            sprintf(buff, "0");
            write(sock, (void *)buff, strlen(buff));
            len = read(sock, (void *)conferma, 16);
            return;
    }
  //se il file di apre invio il suo nome 
    sprintf(buff, "%s",file);
    write(sock, (void *)buff, strlen(buff));

    len = read(sock, (void *)conferma,16);

    while (1) {
         n=read(fd,&buff,600);
         printf("%d\n",n );
    if(n>0) {
         //printf("%s\n",buff);
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

    close(fd);

}



void inviook(int sock){
        char conferma[16];
        sprintf(conferma,"ok");
        write(sock, (void *)conferma, strlen(conferma));

}
