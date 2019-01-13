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

int sock;

void *workerthread(void *arg);
void stampafile(int sock);
int main(){
	pthread_t wr;
	char buff[200];
    char user[30];
	struct sockaddr_in daddr;
	int len;
	sock=socket(AF_INET,SOCK_STREAM,0);

	daddr.sin_family=AF_INET;
	daddr.sin_port=htons(7777);
	daddr.sin_addr.s_addr=inet_addr("127.0.0.1");

	if(connect(sock,(struct sockaddr*)&daddr,sizeof(daddr))==-1){
		perror("Errore ellòa connect");
		close(sock);
		exit(EXIT_FAILURE);
	}
	//pthread_create(&wr,NULL,workerthread,NULL);
	len = read(sock, (void *)buff, 199);
    if(len > 0)
	buff[len] = '\0';
    printf("%s", buff);
    fgets(buff, 199, stdin);

    write(sock, (void *)buff, strlen(buff)); 
    strcpy(user, buff);
    len = read(sock, (void *)buff, 199);
    if(len > 0)
	buff[len] = '\0';
    printf("%s", buff);
    fgets(buff,199, stdin);
    write(sock, (void *)buff, strlen(buff));
if(strncmp(buff,"listhome ",9)){
	printf("----Lista dei file-----\n");
	stampafile(sock);
}}
void stampafile(int sock) {
     char buff[500];
    int len;

    while(1) {

	len = read(sock, (void *)buff, 500);
    	if(len > 0)
            buff[len] = '\0';


	if(strncmp(buff, "finito",6) == 0) {
	    sprintf(buff, "ok");
	    write(sock, (void *)buff, strlen(buff));
	    break;
	}
	printf("%s\n", buff);
	sprintf(buff, "ok");
	write(sock, (void *)buff, strlen(buff)); 

    }    	

}
