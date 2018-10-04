#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#define SA struct sockaddr;


int crea(){
	struct sockaddr_in srv_addr;
	int ret, sk;
	//creo il socket
	sk = socket(AF_INET, SOCK_STREAM, 0);
	//resetto srv_addr
	memset(&srv_addr, 0, sizeof(srv_addr));
	//setto srv_addr
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(1234);
	ret = inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);
	//faccio la connect (connetto il client al server)
	ret = connect(sk, (struct sockaddr *) &srv_addr, sizeof(srv_addr));
	
	return sk;
}


int main(int argc,char* argv[]){
	int sk, ret;
	int lun;
	char msg[1024];

	sk = crea();
	//invio un messaggio
	if(argc>1){
		if(!strcmp(argv[1],"exit"))
			strcpy(msg,"exit");
		else
			strcpy(msg,argv[1]);
	}
	else
		strcpy(msg,"Messaggio di default");
	lun = strlen(msg);
	printf("Byte da inviare: %d\n", lun);
	//invio prima un intero "lun" che indica da quanti byte è composto il messaggio
	ret = send(sk,(void*)&lun,4,0);
	if(ret == -1 || ret < 4){
		printf("Messaggio non inviato");
	}
	else{
		//invio il messaggio vero e proprio "msg"
		ret = send(sk,(void*)msg,strlen(msg),0);
		if(ret == -1 || ret < strlen(msg)){
		printf("Messaggio non inviato");
		}
		else{
		printf("Messaggio inviato con successo: %s \n", msg);
		}
	}
	close(sk);
	return 0;
}
