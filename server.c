#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

//#define SA struct sockaddr;




int crea(){
	struct sockaddr_in my_addr, cl_addr;
	int ret, sk;
	//creo il socket
	sk = socket(AF_INET, SOCK_STREAM, 0);
	//resetto my_addr
	memset(&my_addr, 0, sizeof(my_addr));
	//setto my_addr
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(1234);
	inet_pton(AF_INET, "127.0.0.1", &my_addr.sin_addr.s_addr);
	//faccio la bind (associo il socket sk a my_addr)
	//ret = bind(sk,(SA *)&my_addr,sizeof(my_addr));
	ret = bind(sk, (struct sockaddr *) &my_addr, sizeof(my_addr));
	//faccio la listen (mi metto in ascolto di al più 10 client)
	ret = listen(sk,10);

	return sk;
}

int main(){
	struct sockaddr_in cl_addr;
	int sk, len, sn_sk, ret;
	int lun;
	int exitCond = 0;
	int i;
	char msg[1024];
	sk = crea();
	printf("In attesa di connessioni... \n");
	len = sizeof(cl_addr);
	while(!exitCond){
		//faccio la accept
		sn_sk = accept(sk,(struct sockaddr *) &cl_addr, (socklen_t *) &len);
		//ricevo prima la lunghezza "lun" del messaggio ricevuto
		ret = recv(sn_sk, (void*)&lun,4, 0);
		printf("byte da leggere : %d\n", lun);
		//ricevo il messaggio vero e proprio
		ret = recv(sn_sk, (void*)msg,lun, 0);
		printf("Numero di byte ricevuti = %d\n", ret);
		if((ret == -1) || (ret < lun))
			printf("Errore in ricezione\n");
		else{
			//sistemo la lunghezza della stringa
			msg[ret] = '\0';
			if(!strcmp(msg,"exit"))
				exitCond=1;
			else
				printf("Messaggio ricevuto: %s \n", msg);
		}	
		//chiudo il socket temporaneo (quello della accept())
		close(sn_sk);
	}
	printf("Server terminato\n");
	//chiudo il socket vero e proprio (quello della socket())
	close(sk);
	return 0;
}