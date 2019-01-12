/* Grasso Gianluca 046001897 */

/* Software che dato un path scansiona tutti i file e cartelle ricorsivamente e mette il nome dei file trovati in una lista */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>

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
void concatlis(listafile lis, listafile *main_list);
listafile trovadir();

//dichiarazione lista globale
listafile list;
pthread_mutex_t mutexlista; // mutex che protegge lista globale 

pthread_mutex_t mutexuscita;
int *controllouscita; //variabile di controllo per la terminazione dei thread


int main(int uarg, char **args) {
  int i=0;

    if(uarg == 1) {
    printf("\nDevi inserire il path della directory\n");
     printf("\nQuindi dopo aver digitato ./<nomeeseguibile> scrivi il path\n");
    printf("\nEsempio: ./consegna1 /home/utente/ \n");
    exit(EXIT_FAILURE);
    }
    //Ho trovato su internet un sistema migliore accedere al numero di core tramite la funzione get_nprocs() definita in sys/sysinfo.h>,questa funzione ci ritorna il numero di core
    const int corenumber = get_nprocs();
    controllouscita=(int *)malloc(corenumber*sizeof(int)); 
    pthread_t idthread[corenumber];
    //inizializzazione mutex
    pthread_mutex_init(&mutexlista, NULL);
    pthread_mutex_init(&mutexuscita, NULL);

    printf("\n---- Il numero di core e' %d ----\n", corenumber);
	
    //Se alla fine del path c'e' / lo rimuovo
    if(strlen(args[1])!=1 && args[1][strlen(args[1])-1] == '/') 
	args[1][strlen(args[1])-1]='\0';


    printf("\n---- Inizio il listing delle directory e subdirectory di: %s ----\n", args[1]);
    setNullList(&list);
     ls_directory(&list, args[1]); // lista la directory passata come argomento

    for(i=0; i<corenumber; i++)
        controllouscita[i]=0;

    //Creo la pool di thread: Un thread per ogni core
    for(i=0; i<corenumber; i++) {
	if(pthread_create(&idthread[i], NULL, thread_function, (void *)(intptr_t)i) != 0) { 
	    printf("CREAZIONE THREAD FALLITA\n");
	    exit(EXIT_FAILURE); 
	}	
    }
    printf("\nRicerca file in corso");
	fflush(stdout);
    //Prima di uscire il main thread aspetta che finiscono gli altri thread
    for(i=0; i<corenumber; i++){
	if(pthread_join(idthread[i],NULL) != 0) {
	    printf("Join Fallita\n");
	    exit(EXIT_FAILURE);
	}
    }

    stampalista(list);    
    pthread_mutex_destroy(&mutexlista);
    pthread_mutex_destroy(&mutexuscita);
        exit(EXIT_SUCCESS);

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
