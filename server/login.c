/*
 * Felisberto, Sensoy & Wünsch
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * login.c: Implementierung des Logins
 */

#include "login.h"
#include "common/util.h"
#include "rfc.h"
#include "score.h"
#include "clientthread.h"
#include "catalog.h"
#include "user.h"

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
/*	Funktionen die im File aufgerufen werden:				
 *
 *	login_handler()		-> Ist die Thread-Funktion. Hier wird der Server-Socket gebaut der in einer Endlosschleife läuft, bis das Spiel beendet wird.	
 * 				-> liegt in diesem File 
 *
 *	readMessage()		-> Wertet die Erste Nachricht des Client aus, also den Login-Request. Ist der Name nicht vergeben wird ein neuer Client-Thread erstellt.
 *				-> liegt in diesem File 		
 * 
 *	checkPlayerName()	-> Prüft ob der angegebene Name zu lang oder vergeben ist. Falls ja, wird der Client mit einer Fehlermeldung abgewießen.
 *				-> liegt in diesem File 						
 * 
*/

int pid_file;

void *login_handler(void* noParam){
	
	int retVal=5;
	
	int playerCounter=0;
	int server_socket;
  	int client_socket;
	ssize_t byteCount;
   	char client_message[2000];

	pthread_t user_thread;
	
  	struct sockaddr_in server;
  	struct client;

  	server_socket=socket(AF_INET, SOCK_STREAM, 0);
	allsockets.server_socket=server_socket;
  
  	if(server_socket<0){
    		errorPrint("Fehler beim Erstellen des Sockets");
		return 0;
  	}
	  
  	server.sin_family=AF_INET;		/*IPV4*/
  	server.sin_addr.s_addr=INADDR_ANY;	/*Alle Adressen akzeptieren*/
  	server.sin_port=htons(myconfig.port);	/*Port anwenden, entweder Parameter beim Start oder Default im myconfig Struct*/
  
	/*Socket an die obige Adresse binden*/
  	if(bind(server_socket,(struct sockaddr *)&server, sizeof(server))<0){
    		errorPrint("Fehler beim binden des Sockets, Port vergeben?");
		
		if(!cleanUpSystem()){
			errorPrint("Fehler beim Aufräumen des Systems, Programm wird gekillt.");
			exit(0);
		}	
		printf("\n");
		infoPrint("***********************************************************");
		infoPrint("* System wurde erfolgreich aufgeräumt , Main wird beendet.*");
		infoPrint("***********************************************************");
		printf("\n");
    		return 0;
  	}

	/* Socket wartet nun auf Connections, Backlog 4 heißt? */
  	if(listen(server_socket,4)){
   	 	errorPrint("Fehler beim Listening");
		return 0;
  	}	
  	

 	while(1){		
   		client_socket = accept(server_socket, NULL, NULL);
    		if(client_socket<0){
			errorPrint("Fehler beim akzeptieren des Sockets");
			return 0;
    		}
		if(mutex_struct.sem==0){
			errorPrint("Maximale Spieleranzahl erreicht,  Spieler wird abgewießen.");
			sendERR(1,"Maximale Spieleranzahl erreicht.", client_socket);
			close(client_socket);
			continue;
		}
		if(mutex_struct.sem==-1){
			errorPrint("Spiel läuft bereits, Spieler wird abgewießen.");
			sendERR(1,"Spiel läuft bereits.", client_socket);
			close(client_socket);
			continue;
		}
		
		/* Hier werden jetzt die Daten gelsen die über den Socket versendet werden. */
		
    		byteCount = recv(client_socket, client_message, sizeof(client_message),0);  
		
   		if(byteCount<0){
			errorPrint("Fehler beim lesen!");
			return 0;
    		}
			
		
			
		/* readMessage liegt in der login.h hier wird auf Validität der User-Daten geprüft.*/
		if(readMessage(client_message, client_socket, server_data)){
			/* 	Wenn alles geklappt hat wird hier der Client-Thread erzeugt. 

				Erster Parameter ist der pthread Datentyp, befindet sich in der server_data Struct in der rfc.h 
				Zweiter Datentyp eigentlich immer NULL, weiß nicht was der bringen soll
				Dritter Parameter die Thread-Handler-Funktion, in der clientthread.c
				Vierter Parameter was man will, ich den Client-Socket um nachher eindeutig zu wissen wer Disconnected
			*/
			/*if(pthread_create(&(server_data[playerCounter].user_thread), NULL, connection_handler, (void*) &client_socket)<0){
				errorPrint("Fehler beim Erstellen des Client-Threads.");
				return 0;
			}*/	
			
			if(pthread_create(&user_thread, NULL, connection_handler, (void*) &client_socket)<0){
				errorPrint("Fehler beim Erstellen des Client-Threads.");
				return 0;
			}	
			playerCounter++;
			mutex_struct.sem--;
		}else{
			
		}
		
		/* Kritischer Abschnitt -- START -- Semaphore */
		//pthread_mutex_lock(&mutex_struct.sem_mutex);
		//mutex_struct.sem--;
		//pthread_mutex_unlock(&mutex_struct.sem_mutex);
		/* Kritischer Abschnitt -- ENDE -- Semaphore */
		//playerCounter++;
	}	
	
	
 
 //close(server_socket);
 infoPrint("Login-Thread wird beendet.");
 pthread_exit(NULL);
}


int readMessage(char client_message[], int client_socket, sd server_data[]){	
	int activePlayers=getPlayerAmount(server_data);
	/* Auf den Type der Message prüfen, muss 1 sein da es das erste Paket ist */
	if(client_message[0]==1){
		if(myconfig.recieve){
			debugHexdump(client_message, sizeof(client_message), "C==>S");
		}
	/* prüfen ob der Name valide ist -> vergeben oder maximale Spieleranzahl */
		int valid=checkPlayerName(client_message, server_data, activePlayers);
		if(valid==-1){
			sendERR(1,"Dieser Name ist bereits vergeben.", client_socket);
			return -1;
		}
		if(valid==-2){
			sendERR(1,"Maximale Spieleranzahl erreicht.", client_socket);
			return -1;
		}
		/* In der allsockets Struct steht für jeden Client der Socket-Deskriptor -> bei ID=0, wär es clientSocket[0] usw. */
		allsockets.clientSocket[activePlayers] = client_socket;
		playerdata[activePlayers].socket = client_socket;
		server_data[activePlayers].socket = client_socket;
		sendLOK(activePlayers, client_socket);
		/* Kritischer Abschnitt -- START -- Playerlist*/
		pthread_mutex_lock (&mutex_struct.playerlist_mutex);
		sem_post(&score_semaphore);
		//sendLST(activePlayers);
		pthread_mutex_unlock (&mutex_struct.playerlist_mutex);
		/* Kritischer Abschnitt -- ENDE -- Playerlist*/	
	}
return 1;
}



int checkPlayerName(char client_message[], sd server_data[], int activePlayers){
	char playername[MAX_NAME];
	int j=0;
	for(int i=4;i<client_message[2]+3;i++){
		playername[j]=client_message[i];
		j++;
	}	
	playername[j]='\0';
	if(strcmp(playername , "-p")==0){
		errorPrint("Spielername muss angegeben werden.");
		return -4;
	}

	for(int i=0;i<4;i++){
		if(strcmp(playername,server_data[i].username)==0){
			/* Spielername ist bereits vergeben. */		
			errorPrint("Spielername ist bereits vergeben.");
			return -1;
		}	
	}
	if(server_data[3].username[0]!='\0'){
		/* Maximale Spieleranzahl ereicht. */
		errorPrint("Maximale Spieleranzahl erreicht.");
		return -2;
	}
	
	strcpy(server_data[activePlayers].username, playername);
	server_data[activePlayers].clientID=activePlayers;
	
	return 1;
	
}

int getPlayerAmount(sd server_data[]){
	int userCount=0;
	for(int i=0;i<MAX_PLAYERS;i++){
		if(server_data[i].username[0]!='\0'){
			userCount++;
		}
	}
	return userCount;
}

void druckeFehler(){
	errorPrint("Aufruf: ./server -c KATALOGE [-l LOADER] [-p PORT] [-d] [-m] [-r] [-s]\n");
 	errorPrint("[] -> Optional");
	errorPrint("-c = Catalogs (Pfad zu den Katalogen))");
	errorPrint("[-l] = Loader (Pfad zum Loader (Default:./loader))");
	errorPrint("[-p] = Port (Default: 54321)");
        errorPrint("[-d] = Debug meldungen");
	errorPrint("[-m] = Keine Farben für die Ausgabe auf der Konsole verwenden");
	errorPrint("[-r] = Empfangene Pakete als Hexdump ausgeben");
	errorPrint("[-s] = Gesendete Pakete als Hexdump ausgeben");
}

void setDefaults(){	
	signal(SIGINT, killHandler);
	myconfig.port=54321;
	strcpy(myconfig.catalogs,"");
 	//strcpy(myconfig.catalogs,"../Fragekatalog");
	strcpy(myconfig.loader , "./loader");
	myconfig.debug=0;
	myconfig.monochrome=0;
	myconfig.recieve=0;
	myconfig.send=0;
	 
	infoPrint("Server Gruppe 16");
	infoPrint("Felisberto, Sensoy & Wuensch");
	infoPrint("____________________________");

	catalogs.currentCat[0]='\0';

	mutex_struct.sem=4;
	catalogs.currentCat[0]='\0';
	score_s.score=-1;
	server_data[0].finPlayers=0;
	server_data[0].gameRunning=0;

	/* Das erste Byte aller Usernamen als Ende markieren, das ganze Programm baut darauf auf und prüft immer auf das erste Byte */
	for(int i=0;i<4;i++){
		server_data[i].username[0]='\0';
		player_ranking[i].score=0;
		playerlist[i].score=0;
	}
}

int startThreads(){
	if(pthread_create(&login_thread, NULL, login_handler, NULL)<0){
		errorPrint("Fehler beim Login-Thread.");
		return -1;
	}
	infoPrint("Der Login-Thread wurde erfolgreich gestartet.");

	if(pthread_create(&score_thread, NULL, score_handler, NULL)<0){
		errorPrint("Fehler beim Score-Thread.");
		return -1;
	}
	infoPrint("Der Score-Thread wurde erfolgreich gestartet.");
	return 1;
}

int lockFileExists(){
	pid_file = open("myLockFile", O_CREAT | O_EXCL | O_RDWR, 0666);
	int rc = flock(pid_file, LOCK_EX | LOCK_NB);
	
	if(pid_file < 0){
		return TRUE;
	}else{
		return FALSE;
	}
}

int removeFile(){
	int remRet = remove("myLockFile");
	close(pid_file);
	return remRet;
}

void killHandler(int sig){
	if(cleanUpSystem()<0){
		errorPrint("Fehler beim Aufräumen des Systems nach dem STRG-C Signal.");
		exit(0);
	}else{	
		printf("\n");
		infoPrint("*************************************************************");
		infoPrint("* System wurde erfolgreich nach dem Kill-Signal aufgeräumt. *");
		infoPrint("*************************************************************");
		printf("\n");
		exit(0);
	}
}

void printPlayerData(){
	errorPrint("_______________________________________________________________________________________________________________________________");
	errorPrint("Die Spieler haben folgende Daten: ");
	int amount = getPlayerAmount(server_data);
	
	for(int i=0;i<amount;i++){
		debugPrint("Spielername : %s		Spieler-ID : %d		Spieler-Punkte : %d		Spieler-Frage-Nr : %d		", playerlist[i].spielername , playerlist[i].id, playerlist[i].score , server_data[i].currentQuestion);	
	}
	errorPrint("_______________________________________________________________________________________________________________________________");
}

int cleanUpSystem(){
	printf("\n");
	infoPrint("*************************************************************");
	infoPrint("* 		  System wird jetzt aufgeräumt. 	      *");
	infoPrint("*************************************************************");
	printf("\n");
	if(removeFile()<0){
		errorPrint("Fehler beim Entfernen des Lock-File.");
		return -1;
	}else{
		infoPrint("Lock-File wurde erfolgreich entfernt.");
	}
	if(server_data[0].gameRunning==TRUE){
		if(removeSharedMemory()<0){
			errorPrint("Fehler beim Entfernen des Shared-Memory-Objekts.");
			return -1;
		}else{
			infoPrint("Shared-Memory-Objekt wurde erfolgreich entfernt.");
		}	
		if(closeSharedMemoryFD()<0){
			errorPrint("Fehler beim Entfernen des Shared-Memory-File-Deskriptors.");
			return -1;
		}else{
			infoPrint("Shared-Memory-File-Deskriptor wurde erfolgreich entfernt.");
		}	
	}
	if(closeAllPipes()<0){
		errorPrint("Fehler beim Schließen der Pipes.");
		return -1;
	}else{
		infoPrint("Pipes wurden erfolgreich geschlosen.");
	}
	closeAllSockets();
	
	
	return 1;
}

