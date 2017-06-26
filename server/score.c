/*
 * Felisberto, Sensoy & Wünsch
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * score.h: Implementierung des Score-Agents
 */

#include "score.h"
#include "common/util.h"
#include "clientthread.h"
#include "catalog.h"
#include "rfc.h"
#include "login.h"
#include "user.h"

#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>

/*	Funktionen die im File aufgerufen werden:				
 *
 *	score_handler()		->	Ist die Thread-Funktion des Score-Thread. Sie läuft in einer Dauerschleife und wird aufgerufen sobald ein Spieler Punkte macht,
 *					das Spiel verlässt, oder ein neuer Spieler das Spiel betritt.
 * 					liegt in diesem File 
 *	sortPlayerRanks()	->	Sortiert die Spielerliste neu falls ein Spieler mehr punkte hat, als der Spieler der einen Rang höher ist als er
 *					liegt in der user.c
 *	getPlayerAmount()	->	Liefer den Wert der aktiven Spieler im Spiel
 *				->	liegt in login.c
 *
*/

void *score_handler(void* socket_desc){
	int activePlayers;
	while(1){
		sem_wait(&score_semaphore);
		if(server_data[0].gameOver){
			pthread_exit(NULL);
		}else{
			activePlayers=getPlayerAmount(server_data)-1;
			sortPlayerRanks(server_data);
			sendLST(activePlayers);
		}
	}
}
