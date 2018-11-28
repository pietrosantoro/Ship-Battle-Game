#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

#define MAXCLIENT 10
//colori per rendere più elegante l'output
#define RESET   "\033[0m"		//codice per resettare il colore
#define RED     "\033[31m"      /* Red */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

typedef struct list_client{
	char username [20];
	char indirizzo [16];
	int sock_index;		//indice di socket di riferimento del client
	int porta;
	int libero;		//1 libero / 0 occupato
	struct list_client * next;
}client_connessi;

int crea(int argc,char* argv[]){
	struct sockaddr_in my_addr;
	int ret;
	int listener;
	int porta;
	char indirizzo [16] = "127.0.0.1";
	//creo il socket
	listener = socket(AF_INET, SOCK_STREAM, 0);
	//resetto my_addr
	memset(&my_addr, 0, sizeof(my_addr));
	if(argc != 2){
		if(argc <= 1)
			printf(BOLDRED"Errore"RESET": Porta non specificata\nInserire porta: ");
		if(argc > 1)
			printf(BOLDRED"Errore"RESET": Troppe porte specificate\nInserire una sola porta: ");
		scanf("%d",&porta);
	}
	else
		porta = atoi(argv[1]);
	//setto my_addr
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(porta);
	inet_pton(AF_INET, indirizzo, &my_addr.sin_addr.s_addr);
	//faccio la bind (associo il socket sk a my_addr)
	ret = bind(listener, (struct sockaddr *) &my_addr, sizeof(my_addr));
	if(ret == - 1)
		printf("errore bind\n");
	//faccio la listen (mi metto in ascolto di al più 10 client)
	ret = listen(listener,10);
	printf("Il server è in ascolto all'indirizzo:" BOLDCYAN " %s " RESET "sulla porta:" BOLDCYAN " %d " RESET "\n",indirizzo,porta);
	return listener;
}

client_connessi* inserimento_coda(client_connessi *cliente, int i,char *array_addr){
	int lunghezza_messaggio;
	int ret;
	char username_tmp [20];							//variabile temporanea che salva lo username del client
	int porta_tmp;
	int usr_porta_dsp = 0;							//0 user e porta disponili \ 1 user non disponibile \ 2 porta non disponibile
	client_connessi *client = cliente;
	if(client == NULL){										//la lista dei client è vuota
		client = malloc(sizeof(client_connessi));
		ret = recv(i,(void*)&lunghezza_messaggio,4,0);		//(devo convertirlo in un intero)ricevo prima la lunghezza del messaggio e lo scrivo in lun
		if(ret == - 1)
			printf("errore recv\n");
		ret = recv(i, (void*)client->username,lunghezza_messaggio, 0);	//ricevo il nome del client connesso
		ret = send(i,(void*)&usr_porta_dsp,4,0);							//mando un ack al client avvisandolo che lo username è disponibile
		ret = recv(i,(void*)&lunghezza_messaggio,4,0);		//ricevo prima la lunghezza del messaggio e lo scrivo in lun
		ret = recv(i, (void*)&client->porta,lunghezza_messaggio, 0);	//ricevo la porta di ascolto UDP del client connesso
		strcpy(client->indirizzo,array_addr);
		client->sock_index = i;
		client->libero = 1;
		client->next = NULL;
		printf(BOLDCYAN"%s"RESET" si è connesso\n", client->username);
		printf(BOLDCYAN"%s"RESET" è libero\n",client->username);
		}
	else{
		ret = recv(i,(void*)&lunghezza_messaggio,4,0);		//(devo convertirlo in un intero)ricevo prima la lunghezza del messaggio e lo scrivo in lun
		ret = recv(i,(void*)username_tmp,lunghezza_messaggio, 0);	//ricevo il nome del client connesso
		ret = recv(i,(void*)&lunghezza_messaggio,4,0);		//ricevo prima la lunghezza del messaggio e lo scrivo in lun
		ret = recv(i, (void*)&porta_tmp,lunghezza_messaggio, 0);	//ricevo la porta di ascolto UDP del client connesso
		client_connessi * current = client;
		client_connessi * prec = current;
		while (current != NULL){
			if(!strcmp(current->username,username_tmp)){
				usr_porta_dsp = 1,
				ret = send(i,(void*)&usr_porta_dsp,4,0);							//mando un nack al client avvisandolo che lo username non è disponibile
				return NULL;
			}
			if(current->porta == porta_tmp){
				usr_porta_dsp = 2;
				ret = send(i,(void*)&usr_porta_dsp,4,0);							//mando un nack al client avvisandolo che lo username non è disponibile
				return NULL;
			}
			prec = current;
			current = current->next;
		}
		//inserire controllo su stesso indirizzo, stessa porta (non serve se lo eseguo su macchine diverse)
		usr_porta_dsp = 0;
		ret = send(i,(void*)&usr_porta_dsp,4,0);							//mando un ack al client avvisandolo che lo username è disponibile
		prec->next = malloc(sizeof(client_connessi));
		strcpy(prec->next->username,username_tmp);
		prec->next->porta = porta_tmp;
		strcpy(prec->next->indirizzo,array_addr);
		prec->next->sock_index = i;
		prec->next->libero = 1;
		prec->next->next = NULL;
		printf(BOLDCYAN"%s"RESET" si è connesso\n", prec->next->username);
		printf(BOLDCYAN"%s"RESET" è libero\n",prec->next->username);
	}
	return client;
}

client_connessi* elimina_client(client_connessi *cliente, int i, int uscita_forzata){
	client_connessi * client = cliente;
	client_connessi * prec = client;
	while (client != NULL && client -> sock_index != i){
		prec = client;
		client = client->next;
	}
	if(cliente == NULL)					//caso di lista vuota (caso in cui il client si disconnette forzatamente prima di registrarsi)
		return NULL;
	if(prec == client)					//se l'elemento da eliminare è in testa
		cliente = cliente->next;
	else
		prec->next = client->next;
	if(uscita_forzata)
		printf(BOLDRED"%s"RESET" disconnesso forzatamente\n",client->username);
	else
		printf(BOLDRED"%s"RESET" disconnesso con successo\n",client->username);
	free(client);
	return cliente;
}

client_connessi* cerca_per_nome(client_connessi *cliente, char *name){
	client_connessi * client = cliente;
	while(client != NULL){
		if(!strcmp(client->username,name))
			return client;				//ho trovato il nome e ritorno il puntatore al client
		client = client->next;
	}
	return NULL;
}

client_connessi* cerca_per_indice(client_connessi *cliente, int sock_index){
	client_connessi * client = cliente;
	while(client != NULL){
		if(client->sock_index == sock_index)
			return client;				//ho trovato il nome e ritorno il puntatore al client
		client = client->next;
	}
	return NULL;
}



int main(int argc,char* argv[]){
	//parametri del client collegato
	struct sockaddr_in cl_addr;
	//array degli indirizzi dei client connessi (massimo 10 client)
	char array_addr[MAXCLIENT][16];
	char indirizzo[16];
	//paramertri per gestire i socket
	int listener, len, sn_sk, ret;
	int lunghezza_messaggio;
	char msg_tmp[30];
	char add_msg[6] = " (io)";
	//condizione per uscire dal while
	int exitCond = 0;
	int i;
	int errore_connect[3] = {0,1,2};					//errore connect, 0 utente corretto, 1 utente inesistente, 2 utente occupato
	//parametri per gestire il numero di client collegati
	int num_client = 0;
	int in_coda[MAXCLIENT];		//variabile che mi dice se il client i-esimo è gia in coda oppure no (1 in coda / 0 non in coda)
	//int usr_corretto[MAXCLIENT];
	int connect_fatta[MAXCLIENT];
	int richiesta [2] = {0,1};		// 0 richiesta al client avversario / 1 risposta al client sfidante
	char risposta[2];					// "Y" se il client ha accettato la richiesta a giocare / "N" se il client ha rifiutato
	//puntatore alla lista di client collegati
	client_connessi *client = NULL;
	client_connessi *avversario;
	client_connessi *io;
	int uscita_forzata;
	int esito;					//esito della partita tra i client, 0 se ha perso 1 se ha vinto
	//creo il socket
	listener = crea(argc,argv);
	//parametri per gestire la select
	fd_set master;
	fd_set read_fds;
	int fdmax;
	//azzero i set
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	//aggiungo il listener ai socket da controllare
	FD_SET(listener,&master);
	fdmax = listener;		//il listener è il socket più alto da controllare
	printf("In attesa di connessioni...\n");
	//printf("listener: %d\n", listener);
	while(!exitCond){
		read_fds = master;
		//printf("faccio la select\n");
		select(fdmax + 1,&read_fds,NULL,NULL,NULL);
		for(i = 0; i <= fdmax; i++){
			//printf("i = %d\n", i);
			if(FD_ISSET(i, &read_fds)){
				//printf("i = %d\n", i);
				if(i == listener){
					//printf("è il listener\n");
					len = sizeof(cl_addr);
					//faccio la accept
					sn_sk = accept(listener,(struct sockaddr *) &cl_addr, (socklen_t *) &len);
					inet_ntop(AF_INET, &(cl_addr.sin_addr), array_addr[sn_sk], INET_ADDRSTRLEN);
					in_coda[sn_sk] = 0;
					//usr_corretto[sn_sk] = 0;
					connect_fatta[sn_sk] = 0;
					//printf("sin_port: %d\n", cl_addr.sin_port);
					//printf("sn_sk: %d\n",sn_sk);
					if(sn_sk == -1)
						printf("Connessione fallita\n");
					else{
						printf("\nConnessione stabilita con il client\n");
						FD_SET(sn_sk, &master);
						if(sn_sk > fdmax)
							fdmax = sn_sk;
					}
				}
				else{											//se non è il listener
					if(in_coda[i] == 0){						//se il client non è ancora stato messo in coda, lo metto in coda
						strcpy(indirizzo,array_addr[i]);		//copio l'indirizzo del client i-esimo collegato, nella variabile indirizzo
						client_connessi * tmp = inserimento_coda(client,i,indirizzo);
						if(tmp == NULL){
							printf(BOLDRED"Errore"RESET" registrazione: username o porta gia in uso\n");
						}
						else{
							client = tmp;					//la registrazione è andata a buon fine, e posso aggiornare il puntatore client;
							//printf("aggiorno client\n");
							in_coda[i] = 1;
							num_client ++;
						}
						printf("numero di client collegati : %d\n",num_client);
					}
					else{
						ret = recv(i,(void*)&lunghezza_messaggio,4,0);		//(devo convertirlo in un intero)ricevo prima la lunghezza del messaggio e lo scrivo in lun
						ret = recv(i, (void*)msg_tmp,lunghezza_messaggio, 0);
						if(ret == - 1)
							printf("errore recv\n");
						if(!strcmp(msg_tmp, "!who")){
							client_connessi * current = client;
							ret = send(i,(void*)&num_client,4,0);			//invio prima il numero di client collegati
							while(current != NULL){
								if (i == current -> sock_index){					//mando al client la lista dei client, compreso lui stesso, con l'aggiunta della stringa "sono io"
									strcpy(msg_tmp,current->username);
									strcat(msg_tmp, add_msg);
									lunghezza_messaggio = strlen(msg_tmp) + 1;
									ret = send(i,(void*)&lunghezza_messaggio,4,0);	// invio prima la lunghezza del messaggio
									ret = send(i,(void*)msg_tmp,lunghezza_messaggio,0);
								}
								else{
									lunghezza_messaggio = strlen(current->username) + 1;
									ret = send(i,(void*)&lunghezza_messaggio,4,0);		// invio prima la lunghezza del messaggio
									ret = send(i,(void*)current->username,lunghezza_messaggio,0);
								}
								ret = send(i,(void*)&current->libero,4,0);			//invio direttamente il messaggio perche è un intero
								current = current->next;
							}
						}
						else if(!strcmp(msg_tmp, "!connect") || connect_fatta[i]){
							if(connect_fatta[i] == 0){								//se non ho ancora fatto connect, leggo anche lo username, se ho gia fatto connect, lo username lo leggo prima (al posto del comando)
								ret = recv(i,(void*)&lunghezza_messaggio,4,0);		//(devo convertirlo in un intero)ricevo prima la lunghezza del messaggio e lo scrivo in lun
								ret = recv(i, (void*)msg_tmp,lunghezza_messaggio, 0);
								connect_fatta[i] = 1;
							}
							avversario = cerca_per_nome(client,msg_tmp);
							io = cerca_per_indice(client,i);
							printf(BOLDCYAN"%s"RESET" chiede di poter giocare con "BOLDCYAN"%s"RESET"\n",io->username,msg_tmp);
							if(!avversario){									//se non ho trovato lo username tra i client registrati
								printf(BOLDRED"%s"RESET" non è registrato\n",msg_tmp);
								ret = send(i,(void*)&errore_connect[1],4,0);
								//usr_corretto[i] = 0;
							}
							else if(!avversario->libero){						//se il client trovato, è gia occupato in un'altra partita
								printf(BOLDRED"%s"RESET" è occupato\n",msg_tmp);
								ret = send(i,(void*)&errore_connect[2],4,0);
								//usr_corretto[i] = 0;
							}
							else{												//il client esiste ed è disponibile
								lunghezza_messaggio = strlen(io->username) + 1;
								ret = send(i,(void*)&errore_connect[0],4,0);					//mando un ack al client per dire che è stato trovato il client ed è libero
								ret = send(avversario->sock_index,(void*)&richiesta[0],4,0);	//mando una richiesta al client trovato, per cihedere se accetta la partita
								ret = send(avversario->sock_index,(void*)&lunghezza_messaggio,4,0);	//mando la lunghezza del nome del client che ha richiesto di giocare, al client avversario
								ret = send(avversario->sock_index,(void*)io->username,lunghezza_messaggio,0);		//mando il nome del client che ha fatto richiesta, al client avversario
								ret = recv(avversario->sock_index,(void*)&risposta,2,0);		//leggo un carattere, "Y" se il client ha accettato, "N" se il client ha rifiutato
								ret = send(i,(void*)&risposta,2,0);
								if(!strcmp(risposta, "N")){										//se il client ha rifiutato la partita, dovrò rifare la connect
									printf(BOLDRED"%s"RESET" ha rifiutato la partita\n",avversario->username);
									connect_fatta[i] = 0;
								}
								if(!strcmp(risposta,"Y"))
								{
									ret = send(i,(void*)&avversario->porta,4,0);					//mando un ack al client per dire che è stato trovato il client ed è libero
									lunghezza_messaggio = strlen(avversario->indirizzo) + 1;
									ret = send(i,(void*)&lunghezza_messaggio,4,0);
									ret = send(i,(void*)avversario->indirizzo,lunghezza_messaggio,0);
									ret = send(avversario->sock_index,(void*)&io->porta,4,0);					//mando un ack al client per dire che è stato trovato il client ed è libero
									lunghezza_messaggio = strlen(io->indirizzo) + 1;
									ret = send(avversario->sock_index,(void*)&lunghezza_messaggio,4,0);
									ret = send(avversario->sock_index,(void*)io->indirizzo,lunghezza_messaggio,0);
									printf(BOLDCYAN"%s"RESET" e "BOLDCYAN"%s"RESET" sono in una partita\n",io->username,avversario->username);
									avversario->libero = 0;
									io->libero = 0;
									connect_fatta[i] = 0;
									connect_fatta[avversario->sock_index] = 0;
								}		//controllare qui
								//usr_corretto[i] = 1;
							}

						}
						else if(!strcmp(msg_tmp,"esco")){
							io = cerca_per_indice(client,i);
							ret = recv(i,(void*)&esito,4,0);	//ricvo dal client l'esito della partita
							if(esito == 1)
								printf(BOLDCYAN"%s"RESET" HA VINTO ed è di nuovo libero\n",io->username);
							else if(esito == 0)
								printf(BOLDRED"%s"RESET" HA PERSO ed è di nuovo libero\n",io->username);
							else
								printf(BOLDRED"%s"RESET" si è disconnesso per inattività ed è di nuovo libero\n",io->username);
							io->libero = 1;
							//printf("esco\n");
						}
						else{												//se entro qui è perche il client i-esimo si è disconnesso
							if(!strcmp(msg_tmp, "!quit"))					//se il client fa "quit", si è disconnesso volutamente
								uscita_forzata = 0;
							else											//altrimenti si è disconnesso forzatamente
								uscita_forzata = 1;
							client = elimina_client(client,i,uscita_forzata);
							num_client --;
							close(i);
							FD_CLR(i,&master);
						}
					}
					/*printf("Client connessi al server:\n");
					client_connessi * current = client;
					while(current != NULL){
						printf("%s ",current->username);
						printf("all'indirizzo: %s\n",current->indirizzo);
						current = current->next;
					}*/
					//close(i);
					//FD_CLR(i,&master);
				}
			}
		}
		//chiudo il socket temporaneo (quello della accept())
		//close(sn_sk);
	}
	printf("Server terminato\n");
	//chiudo il socket vero e proprio (quello della socket())
	close(listener);
	return 0;

}