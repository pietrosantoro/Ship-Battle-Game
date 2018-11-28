#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <netinet/in.h>

#define RIGHE 6
#define COLONNE 6
#define MAXCLIENT 10
#define STDIN 0
#define NUM_NAVI 7

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

typedef struct griglie {
	int griglia[RIGHE][COLONNE];	//0 casella libera / 1 casella occupata / 2 nave affondata / 3 tentativo a vuoto
	int tentativi[RIGHE][COLONNE];	//0 nessun tentativo / 1 tentativo riuscito / 2 tentativo fallito
}griglia;


void stampa_comandi(int in_game){
	printf("Sono disponibili i seguenti comandi:\n");
	printf("!help --> mostra l'elenco dei comandi disponibili\n");
	if(in_game){
		printf("!disconnect --> disconnette il client dall'attuale partita\n");
		printf("!shot square --> fai un tentativo con la casella squadre\n");
		printf("!show --> visualizza la griglia di gioco\n");
	}
	else{
		printf("!who --> mostra l'elenco dei client connessi al server\n");
		printf("!connect username --> avvia una partita con l'utente username\n");
		printf("!quit --> disconnette il client dal server\n");
	}
	printf("\n");
}

void stampa_griglia(griglia *mia_griglia, int navi_mie, int navi_avv, char *avv_name, int avv){
	//controllare l'inserimento della casella 77
	printf("  GRIGLIA MIA");
	int colonne;
	int righe;
	if(avv == 1)											//avv è 1 se devo stampare anche la griglia dell'avversario
		printf("\t\t GRIGLIA DI %s\n",avv_name);
	else
		printf("\n");
	printf(" ");
	for(colonne = 1; colonne <= COLONNE;colonne++)		//stampo i numeri delle colonne
		printf(" %d",colonne);
	if(avv == 1){
		printf("\t\t ");
		for(colonne = 1; colonne <= COLONNE;colonne++)		//stampo i numeri delle colonne
			printf(" %d",colonne);
	}
	printf("\n");
	for(righe = 0;righe < RIGHE;righe++){					
		printf("%d ",righe+1);
		for(colonne = 0;colonne < COLONNE;colonne++){		//stampo la mia griglia
			if(mia_griglia->griglia[righe][colonne] == 0)		//nessuna nave
				printf("- ");
			if(mia_griglia->griglia[righe][colonne] == 1)		//nave presente
				printf(BOLDBLUE"O "RESET);
			if(mia_griglia->griglia[righe][colonne] == 2)		//nave affondata
				printf(BOLDMAGENTA"O "RESET);
			if(mia_griglia->griglia[righe][colonne] == 3)		//tentativo a vuoto dell'avversario
				printf(BOLDRED"X "RESET);
		}
		if(avv == 1){
			printf("\t\t");
			printf("%d ",righe+1);
			for(colonne = 0;colonne < COLONNE;colonne++){		//stampo la griglia dell'avversario
				if(mia_griglia->tentativi[righe][colonne] == 0)		//nessun tentativo
					printf("- ");
				if(mia_griglia->tentativi[righe][colonne] == 1)		//nave affondata
					printf(BOLDMAGENTA"O "RESET);
				if(mia_griglia->tentativi[righe][colonne] == 2)		//tentativo fallito
					printf(BOLDRED"X "RESET);
			}
		}
		printf("\n");
	}
	printf("\n");
	printf("LEGENDA:\n- nessuna nave. "BOLDBLUE"O "RESET "nave presente. "BOLDMAGENTA"O "RESET"nave affondata. "BOLDRED"X "RESET"tentativo a vuoto\n\n");
	if(avv == 1){
		printf("Mie navi ancora rimaste: "BOLDBLUE"%d"RESET"\n",navi_mie );
		printf("Navi di %s ancora rimaste: "BOLDBLUE"%d"RESET"\n\n",avv_name,navi_avv);
	}
	return;
}

griglia * iniz_griglia(griglia *mia_griglia){
	//parametri per i cicli
	int righe;
	int colonne;
	int i;
	//parametri per l'input
	int input_corretto = 0;
	int riga_letta = 0, colonna_letta = 0;
	int riga_tmp,colonna_tmp;
	for(righe = 0;righe < RIGHE; righe++)					//inizializzo la mia griglia e quella dei tentativi a 0 (nessuna nave e nessun tentativo)
		for(colonne = 0;colonne < COLONNE;colonne++){
			mia_griglia->griglia[righe][colonne] = 0;
			mia_griglia->tentativi[righe][colonne] = 0;
		}
	for(i = 1; i <= NUM_NAVI; i++){							//inserisco le navi nella griglia
		input_corretto = 0;
		printf("%d° nave nella casella: \n", i);
		//controllo per il corretto inserimento dell'input
		while(!input_corretto){
			riga_letta = scanf("%1d",&riga_tmp);		//leggo un solo intero per la riga
			colonna_letta = scanf("%1d",&colonna_tmp);	//leggo un solo intero per la colonna
			if(!riga_letta || !colonna_letta){
				printf(BOLDRED"errore"RESET": formato non valido, inserire di nuovo la casella:\n");
				while (getchar() != '\n');				//ripulisco il buffer se ho inserito altre cose
			}
			else if(riga_tmp > RIGHE || colonna_tmp > COLONNE){
				printf(BOLDRED"errore"RESET": casella inesistente, inserire di nuovo la casella\n");
				while (getchar() != '\n');				//ripulisco il buffer se ho inserito altre cose
			}
			else if(mia_griglia->griglia[riga_tmp - 1][colonna_tmp -1] == 1)
				printf(BOLDRED"errore"RESET": casella gia occupata, inserire di nuovo la casella\n");
			else{
				input_corretto = 1;
				while (getchar() != '\n');				//ripulisco il buffer se ho inserito altre cose
				printf("inserisco la nave, riga: "BOLDBLUE"%d"RESET" , colonna: "BOLDBLUE"%d"RESET"\n",riga_tmp,colonna_tmp );
			}
		}
		mia_griglia->griglia[riga_tmp-1][colonna_tmp-1] = 1;	//sottraggo uno perche nella matrice, il primo elemento ha indice [0][0]
		//stampare la griglia
		stampa_griglia(mia_griglia,0,0,0,0);							//ultimi parametri 0, perche mi interessa solo la mia griglia
	}																
	return mia_griglia;
}

int game(char *avv_addr,int avv_port, int mia_port,char* avv_name,int who_i){		//ritorna un intero 0 se ho perso / 1 se ho vinto / 2 se mi sono arreso / 3 se mi sono disconnesso in maniera anomala
	int ret;
	int sk_udp;
	struct sockaddr_in avv;
	struct sockaddr_in io;
	char comando[20];
	int rsp[2] = {2,3};		//2 nave affondata	/	3 tentativo a vuoto
	int rsp_tmp;
	int riga_letta = 0;
	int colonna_letta = 0;
	int riga_tmp = 0;
	int colonna_tmp = 0;
	int input_corretto = 0;
	int in_game = 1;
	int ack = 1;			//ack da mandare all'avversario
	//parametri per la lettura e scrittura
	int lunghezza_comando;
	int navi_mie = NUM_NAVI;
	int navi_avv = NUM_NAVI;
	//parametri per la select
	fd_set master;
	fd_set read_fds;
	int fdmax;
	struct timeval timeout = {60,0};
	//azzero i set
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	//struttura per la griglia
	griglia * mia_griglia = malloc(sizeof(griglia));
	sk_udp = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&io,0,sizeof(io));
	io.sin_family = AF_INET;
	io.sin_port = htons(mia_port);
	inet_pton(AF_INET, "127.0.0.1", &io.sin_addr);
	//inet_pton(AF_INET, "127.0.0.1", &avv.sin_addr);
	ret = bind(sk_udp, (struct sockaddr*)&io, sizeof(io));
	if(ret == -1)
		printf("Errore bind\n");
	memset(&avv, 0, sizeof(avv));
	//setto srv_addr
	//se non sono in locale, ho bisogno di un altro socket per inviare (non va bene lo stesso socket per inviare e ricevere), in questo caso utilizzo sk_udp sia per inviare che per ricevere
	avv.sin_family = AF_INET;
	avv.sin_port = htons(avv_port);
	inet_pton(AF_INET, avv_addr, &avv.sin_addr);
	socklen_t addr_len = sizeof(avv);
	//inserisco il socket udp e lo stdin tra i descrittori da controllare
	FD_SET(sk_udp,&master);		//aggiungo il socket con il quale comunico con l'avversario, tra i socket da controllare con la select
	FD_SET(STDIN,&master);	//aggiungo STDIN tra i descrittori da controllare
	fdmax = sk_udp;
	printf("connessione con "BOLDCYAN"%s"RESET" all'indirizzo %s sulla porta %d\n\n",avv_name,avv_addr,avv_port);
	printf("Posiziona %d navi:\n",NUM_NAVI);
	printf("Inserire 2 interi per indicare la casella.\nIl primo indica la riga, il secondo la colonna (es. 34 o 3 4 indica la riga 3 e la colonna 4)\n");
	//inizializzazione della griglia
	mia_griglia = iniz_griglia(mia_griglia);
	ret = sendto(sk_udp,(void*)&ack,4,0,(struct sockaddr*)&avv,sizeof(avv));		//mando un ack all'avversario per dire che ho completato l'inizializzazione della griglia
	if(ret == - 1)
		printf("Errore send\n");
	printf("\nattendere... "BOLDCYAN"%s"RESET" sta ancora posizionando le sue navi\n",avv_name);
	ret = recvfrom(sk_udp, (void*)&ack,4,0, (struct sockaddr *)&avv,&addr_len);		//ricevo l'ack dall'avversario che mi conferma la sua inizializzazione
	//gestire il caso in cui si inserisce nello STDIN qualcosa prima di ricevere l'ack dall'avversario
	//while (getchar() != '\n' || feof(STDIN));				//ripulisco il buffer se ho inserito altre cose
	printf("\n"BOLDCYAN"%s"RESET" è pronto, potete iniziare a giocare\n",avv_name);

	stampa_comandi(in_game);
	while(in_game){
		if(who_i == 0)						//inizia lo sfidante /
			printf("\nE' il tuo turno\n");
		else
			printf("\nE' il turno di "BOLDCYAN"%s"RESET"\n",avv_name);
		printf("# ");
		fflush(stdout);				//mi serve a stampare "#" senza	\n
		read_fds = master;
		//printf("faccio la select\n");
		select(fdmax + 1,&read_fds,NULL,NULL,&timeout);
		if(FD_ISSET(STDIN, &read_fds)){			//faccio la mia mossa e la mando all'avversario
			//printf("input\n");
			scanf("%s",comando);
			printf("\n");
			if(!strcmp(comando,"!help"))
				stampa_comandi(in_game);
			else if(!strcmp(comando,"!disconnect")){
				lunghezza_comando = strlen(comando) + 1;
				ret = sendto(sk_udp,(void*)&lunghezza_comando,4,0,(struct sockaddr*)&avv,sizeof(avv));
				ret = sendto(sk_udp,(void*)comando,lunghezza_comando,0,(struct sockaddr*)&avv,sizeof(avv));
				printf("Disconnessione avvenuta con successo: "BOLDRED"TI SEI ARRESO"RESET"\n\n");
				close(sk_udp);
				return 0;

			}
			else if(!strcmp(comando,"!shot")){
				if(who_i == 0){
					input_corretto = 0;
					while(!input_corretto){
						riga_letta = scanf("%1d",&riga_tmp);		//leggo un solo intero per la riga
						colonna_letta = scanf("%1d",&colonna_tmp);	//leggo un solo intero per la colonna
						if(!riga_letta || !colonna_letta){
							printf(BOLDRED"errore"RESET": formato non valido, inserire di nuovo la casella:\n");
							while (getchar() != '\n');				//ripulisco il buffer se ho inserito altre cose
						}
						else if(riga_tmp > RIGHE || colonna_tmp > COLONNE){
							printf(BOLDRED"errore"RESET": casella inesistente, inserire di nuovo la casella\n");
							while (getchar() != '\n');				//ripulisco il buffer se ho inserito altre cose
						}
						else if(mia_griglia->tentativi[riga_tmp - 1][colonna_tmp - 1] != 0){
							printf(BOLDRED"errore"RESET": tentativo gia effettuato, inserire di nuovo la casella\n");
							while (getchar() != '\n');				//ripulisco il buffer se ho inserito altre cose
						}
						else{
							input_corretto = 1;
							while (getchar() != '\n');				//ripulisco il buffer se ho inserito altre cose
							printf("Sparo in riga: "BOLDBLUE"%d"RESET", colonna: "BOLDBLUE"%d"RESET"\n",riga_tmp,colonna_tmp );
						}
					}
					lunghezza_comando = strlen(comando) + 1;
					ret = sendto(sk_udp,(void*)&lunghezza_comando,4,0,(struct sockaddr*)&avv,sizeof(avv));		//invio la lunghezza del comando all'avversario
					ret = sendto(sk_udp,(void*)comando,lunghezza_comando,0,(struct sockaddr*)&avv,sizeof(avv));	//invio il comando all'avversario
					ret = sendto(sk_udp,(void*)&riga_tmp,4,0,(struct sockaddr*)&avv,sizeof(avv));				//invio la riga che intendo colpire
					ret = sendto(sk_udp,(void*)&colonna_tmp,4,0,(struct sockaddr*)&avv,sizeof(avv));			//invio la colonna che intendo colpire
					who_i = 1;
					ret = recvfrom(sk_udp, (void*)&rsp_tmp,4,0, (struct sockaddr *)&avv,&addr_len);
					if(rsp_tmp == 2){
						printf(BOLDMAGENTA"COLPITO\n"RESET);
						mia_griglia->tentativi[riga_tmp - 1][colonna_tmp - 1] = 1;
						navi_avv --;
						if(navi_avv == 0){
							printf("Hai affondato tutte le navi, complimenti: "BOLDMAGENTA"HAI VINTO"RESET"\n\n");
							close(sk_udp);
							return 1;
						}
					}
					if(rsp_tmp == 3){
						printf(BOLDRED"MANCATO\n"RESET);
						mia_griglia->tentativi[riga_tmp - 1][colonna_tmp - 1] = 2;
					}

				}
				else{
					printf("Non è il tuo turno, aspetta che "BOLDCYAN"%s"RESET" faccia la sua mossa\n",avv_name);
				}
			}
			else if(!strcmp(comando,"!show")){
				stampa_griglia(mia_griglia,navi_mie,navi_avv,avv_name,1);					//1 perche devo stampare anche la griglia dell'avversario
			}
			else
				printf("comando non valido, inserirne un altro\n");
		}
		else if (FD_ISSET(sk_udp, &read_fds)){		//l'avversario ha fatto la sua mossa e la ricevo
			//printf("socket\n");
			ret = recvfrom(sk_udp, (void*)&lunghezza_comando,4,0, (struct sockaddr *)&avv,&addr_len);	//ricevo la lunghezza del comando dall'avv
			ret = recvfrom(sk_udp, (void*)comando,lunghezza_comando,0, (struct sockaddr *)&avv,&addr_len);	//ricevo il comando dall'avv
			if(!strcmp(comando,"!disconnect")){
				printf("\n"BOLDCYAN"%s"RESET" si è arreso, complimenti: "BOLDMAGENTA"HAI VINTO"RESET"\n\n", avv_name);
				close(sk_udp);
				return 1;
			}
			if(!strcmp(comando,"!shot")){
				ret = recvfrom(sk_udp, (void*)&riga_tmp,4,0, (struct sockaddr *)&avv,&addr_len);		//ricevo la riga che l'avv intende colpire
				ret = recvfrom(sk_udp, (void*)&colonna_tmp,4,0, (struct sockaddr *)&avv,&addr_len);		//ricevo la colonna che l'avv intende colpire
				if(mia_griglia->griglia[riga_tmp - 1][colonna_tmp - 1] == 1){									//l'avversario ha colpito la mia nave
					printf("\n"BOLDCYAN"%s"RESET" spara in riga : %d, colonna: %d,"BOLDMAGENTA" COLPITO\n"RESET,avv_name,riga_tmp,colonna_tmp);
					mia_griglia->griglia[riga_tmp - 1][colonna_tmp - 1] = 2;									//l'avversario ha affondato la mia nave
					ret = sendto(sk_udp,(void*)&rsp[0],4,0,(struct sockaddr*)&avv,sizeof(avv));
					navi_mie --;
					if(navi_mie == 0){
						printf("\n"BOLDCYAN"%s"RESET" ha affondato tutte le tue navi : "BOLDRED"HAI PERSO"RESET"\n\n",avv_name);
						close(sk_udp);
						return 0;
					}
				}
				if(mia_griglia->griglia[riga_tmp - 1][colonna_tmp - 1] == 0){
					printf("\n"BOLDCYAN"%s"RESET" spara in riga : %d, colonna: %d,"BOLDRED" MANCATO\n"RESET,avv_name,riga_tmp,colonna_tmp);
					mia_griglia->griglia[riga_tmp - 1][colonna_tmp - 1] = 3;									//l'avversario non ha affondato la mia nave
					ret = sendto(sk_udp,(void*)&rsp[1],4,0,(struct sockaddr*)&avv,sizeof(avv));
				}
				who_i = 0;

			}
			if(!strcmp(comando,"timeout")){
				printf("\n"BOLDCYAN"%s"RESET" si è disconnesso per inattività, complimenti: "BOLDMAGENTA"HAI VINTO"RESET"\n\n", avv_name);
				close(sk_udp);
				return 1;
			}
			//inserire controllo su disconnessione forzata client
			//ret = recv(sk_udp,(void*)rsp,5,0);
			//printf("risposta dal client: %s\n",rsp);
		}
		else{	//timeout scaduto
			strcpy(comando,"timeout");
			lunghezza_comando = strlen(comando) + 1;
			ret = sendto(sk_udp,(void*)&lunghezza_comando,4,0,(struct sockaddr*)&avv,sizeof(avv));
			ret = sendto(sk_udp,(void*)comando,lunghezza_comando,0,(struct sockaddr*)&avv,sizeof(avv));
			printf("Timeout scaduto: "BOLDRED"HAI PERSO"RESET"\n\n");
			close(sk_udp);
			return 2;

		}
	}
	return 0;
}

int main(int argc,char* argv[]){
	//struttura per i parametri del server
	struct sockaddr_in srv_addr;
	//identificatori di socket
	int ret, sk;
	//parametri di connessione con il server
	int porta;
	char indirizzo[16];
	int exit_cond = 1;
	//numero parametri + 1 (parametro 0)
	int num_param = 3;
	int lunghezza_messaggio, lunghezza_comando, lunghezza_name, lunghezza_porta;
	int i;
	//parametri per la lista di client collegati
	int num_client;
	char name_client[MAXCLIENT][30];
	int client_libero [MAXCLIENT];
	int usr_porta_disponibile = 3;		//0 usr e porta corretti \ 1 usr gia in uso \ 2 porta gia in uso \ 3 mi serve per entrare nel while senza errori
	int richiesta;			//ricevo un intero dal server, se è 0 sono l'avversario, se è 1 sono lo sfidante
	char risposta[2];			//invio la risposta al server, per dire se ho accettato la richiesta a giocare, "Y" se ho accettato / "N" se ho rifiutato
	int risposta_corretta = 0;
	//parametri di gioco
	char my_name [20];		//username massimo 20 caratteri
	int porta_UDP;
	char comando[30];
	int in_game = 0;
	char esco[6] = "esco"; 	//avvisa il server che sono uscito dalla partita
	int esito;				//0 se ho perso 1 se ho vinto 2 se mi sono disconnesso per inattività
	//dati dell'avversario
	char avv_name[20];		//nome dell'avversario
	char avv_addr[20];		//indirizzo ip dell'avversario
	int avv_port;			//porta di ascolto udp dell'avversario
	int avversario_corretto = 1;
	//parametri per gestire la select tra i socket e lo STDIN
	fd_set master;
	fd_set read_fds;
	int fdmax;
	//azzero i set
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	if(argc != num_param){						// se i parametri inseriti sono troppi o troppo pochi
		if(argc == (num_param - 2) || argc > (num_param - 1)){
			if(argc == (num_param - 2))			//se non ho inserito ne l'indirizzo ne la porta
				printf(BOLDRED"Errore"RESET": Indirizzo e porta non specificati\n");
			else								//se ho inserito troppi argomenti, faccio inserire tutto da capo
				printf(BOLDRED"Errore"RESET": Troppi argomenti specificati\n");
			printf("Inserire indrizzo: ");
			scanf("%s",indirizzo);
		}
		if(argc == (num_param - 1)){			//se ho inserito solo l'indirizzo, faccio inserire solo la porta
			printf(BOLDRED"Errore"RESET": Porta non specificata\n");
			strcpy(indirizzo, argv[1]);
		}
		printf("Inserire porta: ");
		scanf("%d",&porta);
	}
	else{
		strcpy(indirizzo, argv[1]);
		porta = atoi(argv[2]);
	}
	//creo il socket
	sk = socket(AF_INET, SOCK_STREAM, 0);
	FD_SET(sk,&master);		//aggiungo il socket con il quale comunico con il server, tra i socket da controllare con la select
	FD_SET(STDIN,&master);	//aggiungo STDIN tra i descrittori da controllare
	fdmax = sk;
	//resetto srv_addr
	memset(&srv_addr, 0, sizeof(srv_addr));
	//setto srv_addr
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(porta);
	ret = inet_pton(AF_INET, indirizzo, &srv_addr.sin_addr);
	//faccio la connect (connetto il client al server)
	ret = connect(sk, (struct sockaddr *) &srv_addr, sizeof(srv_addr));
	if(ret == -1){
		printf("Connessione fallita\n");
		close(sk);
		return 0;
	}
	else{
		printf("Connessione al server "BOLDCYAN"%s"RESET" sulla porta "BOLDCYAN"%s"RESET" riuscita\n\n",indirizzo,argv[2]);
		stampa_comandi(in_game);
		while(usr_porta_disponibile){
			if(usr_porta_disponibile == 1 || usr_porta_disponibile == 3){  //inserisco lo usr se non l'ho ancora inserito o se ne ho inserito uno gia in uso
				printf("Inserisci il tuo nome (max 20 caratteri): ");
				scanf("%s",my_name);
				lunghezza_name = (strlen(my_name) + 1);
			}
			if(usr_porta_disponibile == 2 || usr_porta_disponibile == 3){ //inserisco la porta se ancora non l'ho inserita o se ne ho inserita una gia in uso
				printf("Inserisci la porta UDP di ascolto: ");
				scanf("%d",&porta_UDP);
				printf("\n");
				lunghezza_porta = (sizeof(porta_UDP) + 1);		//devo convertirlo in un buffer
			}
			ret = send(sk,(void*)&lunghezza_name,4,0);		// invio prima la lunghezza del messaggio
			ret = send(sk,(void*)my_name,lunghezza_name,0);
			ret = send(sk,(void*)&lunghezza_porta,4,0);		// invio prima la lunghezza del messaggio
			ret = send(sk,(void*)&porta_UDP,lunghezza_porta,0);
			//ricevo un ack dal server che mi dice se lo username è disponibile oppure no
			ret = recv(sk,(void*)&usr_porta_disponibile,4,0);
			if(usr_porta_disponibile == 1)
				printf(BOLDRED"Errore"RESET" :Username già in uso, inserirne un altro\n\n");
			else if(usr_porta_disponibile == 2)
				printf(BOLDRED"Errore"RESET" :Porta UDP già in uso, inserirne un'altra\n\n");
			else
				printf("Registrazione avvenuta con successo\n\n");
		}
		while(exit_cond){
			printf("> ");
			fflush(stdout);				//mi serve a stampare ">" senza	\n
			read_fds = master;
			//printf("faccio la select\n");
			int retval = select(fdmax + 1,&read_fds,NULL,NULL,NULL);
			if(retval == -1)
				printf("errore select\n");
			if (FD_ISSET(STDIN, &read_fds)){
			//	printf("input\n");
				scanf("%s",comando);
				printf("\n");
				lunghezza_comando = strlen(comando) + 1;
				if(!strcmp(comando,"!help"))
					stampa_comandi(in_game);
				else if(!strcmp(comando,"!who")){

					ret = send(sk,(void*)&lunghezza_comando,4,0);		// invio prima la lunghezza del messaggio
					ret = send(sk,(void*)comando,lunghezza_comando,0);
					//ricevo i client collegati al server
					ret = recv(sk,(void*)&num_client,4,0);		//ricevo prima il numero di client collegati

					for (i = 0;i < num_client; i++){
						ret = recv(sk,(void*)&lunghezza_messaggio,4,0);		//(devo convertirlo in un intero)ricevo prima la lunghezza del messaggio e lo scrivo in lun
						ret = recv(sk, (void*)&name_client[i],lunghezza_messaggio, 0);	//ricevo il nome del client connesso
						ret = recv(sk, (void*)&client_libero[i],4, 0);	//ricevo un intero che indica se il client è libero oppure no
					}
					printf("Client connessi al server: \n");
					//stampa i client connessi al server
					for(i = 0;i < num_client; i++){
						printf("%s \t\t: ",name_client[i]);

						if(client_libero[i])
							printf(BOLDCYAN"\t(libero)\n"RESET);
						else
							printf(BOLDRED"\t(occupato)\n"RESET);
					}
					printf("\n");
				}
				else if(!strcmp(comando,"!connect")){		//gestire il caso in cui scrivo connect senza il nome dell'avversario
					ret = send(sk,(void*)&lunghezza_comando,4,0);		// invio prima la lunghezza del messaggio
					ret = send(sk,(void*)comando,lunghezza_comando,0);
					avversario_corretto = 1;							//lo metto a 1 per entrare nel while
					while(avversario_corretto != 0){
						scanf("%s",avv_name);
						if(!strcmp(avv_name,my_name))
							printf(BOLDRED"Errore"RESET": non puoi giocare contro te stesso, inserire un altro avversario: ");
						else{
							printf("Mi connetto con: %s\n\n",avv_name);
							lunghezza_name = strlen(avv_name) + 1;
							ret = send(sk,(void*)&lunghezza_name,4,0);		// invio prima la lunghezza del messaggio
							ret = send(sk,(void*)avv_name,lunghezza_name,0);
							//ricevo un ack dal server, 0 utente corretto, 1 utente inesistente, 2 utente occupato,
							ret = recv(sk,(void*)&avversario_corretto,4,0);
							if(avversario_corretto == 1)
								printf(BOLDRED"Errore"RESET": username inesistente, inserirne un altro: ");
							if(avversario_corretto == 2)
								printf(BOLDRED"Errore"RESET": %s è occupato in un'altra partita, inserirne un altro: ",avv_name);
							if(avversario_corretto == 0){
								//printf("avversario corretto\n");
								printf("Aspetto la risposta da: "BOLDCYAN"%s"RESET"\n\n",avv_name);
								ret = recv(sk,(void*)risposta,2,0);
								if(!strcmp(risposta,"Y")){
									ret = recv(sk,(void*)&avv_port,4,0);			//ricevo una richiesta da parte di un client di giocare
									ret = recv(sk,(void*)&lunghezza_messaggio,4,0);			//ricevo la lunghezza del nome del client che ha richiesto di giocare con me
									ret = recv(sk,(void*)avv_addr,lunghezza_messaggio,0);
									printf(BOLDCYAN"%s"RESET" ha accettato la tua richiesta\n",avv_name);
									esito = game(avv_addr,avv_port,porta_UDP,avv_name,0);			//l'ultimo parametro indica se sono lo sfidante o l'avversario (0 sfidante)
									lunghezza_messaggio = strlen(esco) + 1;
									ret = send(sk,(void*)&lunghezza_messaggio,4,0);
									ret = send(sk,(void*)esco,lunghezza_messaggio,0);
									ret = send(sk,(void*)&esito,4,0);								//invio al server l'esito della partita
									in_game = 0;	
								}
								else{
									printf(BOLDRED"%s"RESET" ha rifiutato la tua richiesta\n",avv_name);
								}
							}
						}
					}
				}
				else if(!strcmp(comando,"!quit")){
					ret = send(sk,(void*)&lunghezza_comando,4,0);		// invio prima la lunghezza del messaggio
					ret = send(sk,(void*)comando,lunghezza_comando,0);
					exit_cond = 0;
				}
				else
					printf(BOLDRED"Errore"RESET": comando non valido, inserirne un altro\n");
			}
			else if (FD_ISSET(sk, &read_fds)){
				risposta_corretta = 0;
				//printf("socket\n");
				ret = recv(sk,(void*)&richiesta,4,0);			//ricevo una richiesta da parte di un client di giocare
				ret = recv(sk,(void*)&lunghezza_messaggio,4,0);			//ricevo la lunghezza del nome del client che ha richiesto di giocare con me
				ret = recv(sk,(void*)avv_name,lunghezza_messaggio,0);			//ricevo il nome del client che ha chiesto di giocare con me
				printf("\n"BOLDCYAN"%s"RESET" chiede di avviare una partita con te (Y/N): ",avv_name);
				while(!risposta_corretta){
					//printf("risposta : %s\n",risposta);
					scanf("%s",risposta);
					if(!strcmp(risposta,"Y")){
						risposta_corretta = 1;
						ret = send(sk,(void*)&risposta,2,0);		//mando un solo byte (devo mandare solo un carattere)
						ret = recv(sk,(void*)&avv_port,4,0);			//ricevo una richiesta da parte di un client di giocare
						ret = recv(sk,(void*)&lunghezza_messaggio,4,0);			//ricevo la lunghezza del nome del client che ha richiesto di giocare con me
						ret = recv(sk,(void*)avv_addr,lunghezza_messaggio,0);	//ho accettato la richiesta e sono in gioco
						printf("\nAccetto la richiesta\n");
						esito = game(avv_addr,avv_port,porta_UDP,avv_name,1);					//l'ultimo parametro indica che sono l'avversario
						lunghezza_messaggio = strlen(esco) + 1;
						ret = send(sk,(void*)&lunghezza_messaggio,4,0);
						ret = send(sk,(void*)esco,lunghezza_messaggio,0);
						ret = send(sk,(void*)&esito,4,0);										//invio al server l'esito della partita
					}
					else if(!strcmp(risposta,"N")){
						risposta_corretta = 1;
						ret = send(sk,(void*)&risposta,2,0);		//mando un solo byte (devo mandare solo un carattere)
						printf("\nRifiuto la richiesta\n");
					}
					else{
						printf(BOLDRED"Errore"RESET": carattere non valido, inserire (Y/N): ");
					}
				}
			}
		}
	}
	printf("\nDiscconnessione avvenuta con successo\n");
	close(sk);
	return 0;
}