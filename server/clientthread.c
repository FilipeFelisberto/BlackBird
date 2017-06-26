/*
 * Felisberto, Sensoy & Wünsch
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * clientthread.c: Implementierung des Client-Threads
 */

#include "clientthread.h"
#include "common/util.h"
#include "clientthread.h"
#include "rfc.h"
#include "login.h"
#include "user.h"
#include "score.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>

/*	Funktionen die im File aufgerufen werden:				
 *	
 *	connection_handler()	->	Ist die Thread-Funktion des Client-Thread
 * 				->	liegt in diesem File
 *      isGameLeader()		->	Prüft ob der Spieler der das Spiel verlassen hat der Spieleleiter ist.
 *				->	liegt in user.c
 *	sortPlayerList()	->	Lässt die Spieler nach oben rutschen, wenn ein Spieler das Spiel verlassen hat.
 *				-> 	liegt in user.c
 *	getPlayerAmount()	->	Liefer den Wert der aktiven Spieler im Spiel
 *				->	liegt in login.c
 *	encodeMessage()		->	Wertet die Empfangene Nachricht aus und sorgt für die Antwortpakete.
 *				->	liegt in user.c
*/


void *connection_handler(void* socket_desc){
	
    	int client_socket = *(int*)socket_desc;

    	while(1){
      		uint8_t client_message[2000];
      		int recv_value;
      		recv_value = recv(client_socket, client_message, sizeof(client_message),0); 
			if(recv_value==0){
				if(isGameLeader(client_socket)){
					/* Kritischer Abschnitt -- START -- Sockets */
					pthread_mutex_lock(&mutex_struct.socket_mutex);
					//close(allsockets.server_socket);
					for(int i=0;i<4;i++){
						if(allsockets.clientSocket[i]!=0){
							sendERR(1,"Spielleiter hat das Spiel verlassen.", allsockets.clientSocket[i]);
							if(myconfig.send){
								debugHexdump("Spielleiter hat das Spiel verlassen.", 3, "C==>S");
							}
						//close(allsockets.clientSocket[i]);
						}
					}
					//close(allsockets.server_socket);
					/* Kritischer Abschnitt -- START -- Semaphore */
					pthread_mutex_lock(&mutex_struct.sem_mutex);
					mutex_struct.sem++;
					pthread_mutex_unlock(&mutex_struct.sem_mutex);
					/* Kritischer Abschnitt -- ENDE -- Semaphore */
					pthread_mutex_unlock(&mutex_struct.socket_mutex);
					/* Kritischer Abschnitt -- ENDE -- Sockets*/
					
					server_data[0].gameOver=1;
	
				}
				/* Kritischer Abschnitt -- START -- Playerlist */
				pthread_mutex_lock (&mutex_struct.playerlist_mutex);
				sortPlayerList(client_socket);
				mutex_struct.sem++;
				pthread_mutex_unlock (&mutex_struct.playerlist_mutex);
				/* Kritischer Abschnitt -- ENDE -- Playerlist*/
				int activePlayers = getPlayerAmount(server_data);	
				if(server_data[0].gameOver!=1 && activePlayers < 2 && server_data[0].gameRunning==1){
						errorPrint("Zu wenig aktive Spieler, das Spiel wird beendet.");
						sendERR(1,"Zu wenig aktive Spieler im Spiel, das Spiel wird beendet.", allsockets.clientSocket[0]);
						server_data[0].gameOver=1;
				}
				/* Kritischer Abschnitt -- START -- Playerlist */
				pthread_mutex_lock (&mutex_struct.playerlist_mutex);
				sem_post(&score_semaphore);
				pthread_mutex_unlock (&mutex_struct.playerlist_mutex);
				/* Kritischer Abschnitt -- ENDE -- Playerlist*/
				
				if(server_data[0].finPlayers==activePlayers){
					infoPrint("Der letzte aktive Spieler hat das Spiel verlassen. Ergebnisse werden ausgewertet.");
					sendGOV();
					server_data[0].gameOver=1;
					//close(allsockets.server_socket);
				}
				
				pthread_exit(NULL);
		}
		if(recv_value<0){
			errorPrint("Fehler beim Empfangen der Nachrichten im Client-Thread");
			server_data[0].gameOver=1;
			pthread_exit(NULL);
		}
		encodeMessage(client_message, client_socket);
	}
}

