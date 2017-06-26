/*
 * Felisberto, Sensoy & Wünsch
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * user.c: Implementierung der User-Verwaltung
 */

#include "user.h"
#include "rfc.h"
#include "catalog.h"
#include "score.h"
#include "login.h"

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>

/*	Funktionen die im File aufgerufen werden:				
 *
 *	encodeMessage()		->	Wertet alle eingekommenen Pakete vom Client aus und verschickt anschließend die Antwortpakete. Die Funktion prüft immer auf das
 * 					erste Byte des Empfangenen Paketes um den Type in Erfahrung zu bringen.
 *
 * 	getPlayerAmount()	->	Liefer den Wert der aktiven Spieler im Spiel
 *				->	liegt in login.c
 *
 * 	loadCat()		->	Lädt den angegebenen Katalog in den Shared-Memory, als Parameter der Name des katalogs.
 *				->	liegt in catalog.c
 * 
 * 	createBitmask()		->	Erstellt die Bitmaske, die verschickt werden soll, je nach dem, ob Zeit abgelaufen oder nicht.
 *				->	liegt in diesem File 
 * 
*/

int questionNumber=0;
int timeLeft;

clock_t questionStart;
clock_t questionEnd;
clock_t answerRecieved;

//int finPlayers=0;

void encodeMessage(char message[], int client_socket){
	
	int activePlayers = getPlayerAmount(server_data);
	int thisPlayer;
	for(int i=0;i<4;i++){
		if(server_data[i].socket==client_socket){
			thisPlayer=i;	
		}
	}

	switch(message[0]){
				case CRQ_TYPE:		if(myconfig.recieve){
							debugHexdump(message, 3, "C==>S");
							}
								sendCRS(client_socket);
							if(catalogs.currentCat[0]!='\0'){
								send(client_socket, message, message[2]+3,0);
							}
							break;


				case CCH_TYPE: 		if(myconfig.recieve){
								debugHexdump(message, message[2]+3, "C==>S");
							}
							for(int i=0;allsockets.clientSocket[i]!=0 && i < 4;i++){
								char catalog[50];
								int catEnd;
								for(int i=3;i<message[2]+3;i++){
									catalog[i-3] = message[i];
									catEnd=i-3;
								}
								catalog[catEnd+1]='\0';
								strcpy(catalogs.currentCat, catalog);
								
								if(myconfig.send){
									debugHexdump(message, message[2]+3, "S==>C");
								}
								send(allsockets.clientSocket[i], message, message[2]+3,0);
							}
							break;


				case STG_TYPE: 		if(myconfig.recieve){
								debugHexdump(message, message[2]+3, "C==>S");
							}
							if(allsockets.clientSocket[1]==0){
									sendERR(0,"Es muessen mindestens 2 Spieler angemeldet sein, um das Spiel zu starten!", client_socket);
									break;
							}
							for(int i=0;allsockets.clientSocket[i]!=0 && i < 4;i++){
								if(myconfig.send){
									debugHexdump(message, message[2]+3, "S==>C");
								}
								send(allsockets.clientSocket[i], message, message[2]+3,0);
							}

							/* Kritischer Abschnitt -- START -- Semaphore */
							pthread_mutex_lock(&mutex_struct.sem_mutex);
							mutex_struct.sem=-1;
							pthread_mutex_unlock(&mutex_struct.sem_mutex);
							/* Kritischer Abschnitt -- ENDE -- Semaphore */
							loadCat(catalogs.currentCat);
							for(int i=0;i<4;i++){
								server_data[i].currentQuestion=0;
							}
							sem_post(&score_semaphore);
							server_data[0].gameRunning=1;
							break;
	

				case QRQ_TYPE: 		if(myconfig.recieve){
								debugHexdump(message, 3, "C==>S");
							}
							
							if(server_data[thisPlayer].currentQuestion!=catalogs.questAmount){
								sendQUESTION(client_socket ,server_data[thisPlayer].currentQuestion, thisPlayer); 

								int timerStatus = startTimer(client_socket, thisPlayer);
								if(timerStatus==0){
									int correctBitmask = createBitmask(FALSE, NULL, thisPlayer);
									sendQURS(client_socket, correctBitmask);
									server_data[thisPlayer].currentQuestion++;
								}
								if(timerStatus<0){
									errorPrint("Fehler bei dem Timer!");
								}	
							}else{
								sendLASTQUESTION(client_socket);
								server_data[0].finPlayers++;

								if(server_data[0].finPlayers==activePlayers){
									infoPrint("Das Spiel ist zuende.");
									sendGOV();
									server_data[0].gameOver=1;
									//close(allsockets.server_socket);
								}
							}
							break;

				case QAN_TYPE:	        if(myconfig.recieve){
								debugHexdump(message, 3, "C==>S");
							}
							int correctBitmask = createBitmask(TRUE, message, thisPlayer);
							sendQURS(client_socket, correctBitmask);
							server_data[thisPlayer].currentQuestion++;
							break;

	}
}

int startTimer(int client_socket, int playerIndex){
	int timeout = all_questions[server_data[playerIndex].currentQuestion].timeLimit;
	int retSelect;
	clock_t startzeit = clock();
	clock_t endzeit = startzeit + timeout;
	
	questionStart = startzeit;
	questionEnd = endzeit;
	
	clock_t restzeit = timeout;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(client_socket, &fds);
	
	time_t questionSent = time(0);
	
	retSelect = pselect(client_socket+1 , &fds, NULL, NULL, &restzeit, NULL);
	
	time_t questionRecieved = time(0);
	
	timeLeft = questionRecieved - questionSent;
	

	answerRecieved = clock();
	
	clock_t finalTime = questionEnd - answerRecieved;

	
	return retSelect;
}

void closeAllSockets(){
	int players;
	players=getPlayerAmount(server_data);
	for(int i=0;allsockets.clientSocket[i]!=0 && i < 4 ;i++){
		if(close(allsockets.clientSocket[i])<0 && server_data[0].gameRunning == TRUE){
			errorPrint("Fehler beim Schließen der Client-Sockets.");
		}
	}			
	infoPrint("Die Client-Sockets wurden erfolgreich geschlossen.");
	if(close(allsockets.server_socket)<0){
		errorPrint("Fehler beim Schließen des Server-Sockets.");
	}else{
		infoPrint("Server Socket wurde erfolgreich geschlossen.");
	}
}

int isGameLeader(int client_socket){
	for(int i=0;i<4;i++){
		if(allsockets.clientSocket[i]==client_socket){
			if(playerlist[i].id==0){
			//if(i==0){
				return 1;
			}else{
				return 0;
			}
		}
	}
return 0;
}

void sortPlayerRanks(sd server_data[]){
	char playerName[255];
	char textHolder[255];

	int numbHolder;
	int currentPlayers;
	int playerID=-1;
	currentPlayers=getPlayerAmount(server_data);

	for(int i=0;i<4;i++){
		if(playerlist[i+1].score > playerlist[i].score){
			playerID=i+1;
			break;
		}
	}
	if(playerID!=-1){
		for(int i=0;i<4;i++){
			if(playerlist[playerID].score > playerlist[playerID-1].score && playerID > 0){
				strcpy(textHolder, playerlist[playerID-1].spielername);
				strcpy(playerlist[playerID-1].spielername , playerlist[playerID].spielername);
				strcpy(playerlist[playerID].spielername , textHolder);

				numbHolder = playerlist[playerID-1].score;
				playerlist[playerID-1].score = playerlist[playerID].score;
				playerlist[playerID].score = numbHolder;

				numbHolder = playerlist[playerID-1].id;
				playerlist[playerID-1].id = playerlist[playerID].id;
				playerlist[playerID].id = numbHolder;

				numbHolder = player_ranking[playerID-1].score;
				player_ranking[playerID-1].score = player_ranking[playerID].score;
				player_ranking[playerID].score = numbHolder;

				strcpy(textHolder, server_data[playerID-1].username);
				strcpy(server_data[playerID-1].username , server_data[playerID].username);
				strcpy(server_data[playerID].username , textHolder);

				numbHolder = server_data[playerID-1].clientID;
				server_data[playerID-1].clientID = server_data[playerID].clientID;
				server_data[playerID].clientID = numbHolder;

				numbHolder = server_data[playerID-1].socket;
				server_data[playerID-1].socket = server_data[playerID].socket;
				server_data[playerID].socket = numbHolder;

				numbHolder = server_data[playerID-1].catAmount;
				server_data[playerID-1].catAmount = server_data[playerID].catAmount;
				server_data[playerID].catAmount = numbHolder;

				numbHolder = server_data[playerID-1].currentQuestion;
				server_data[playerID-1].currentQuestion = server_data[playerID].currentQuestion;
				server_data[playerID].currentQuestion = numbHolder;

				numbHolder = server_data[playerID-1].currentBitmaks;
				server_data[playerID-1].currentBitmaks = server_data[playerID].currentBitmaks;
				server_data[playerID].currentBitmaks = numbHolder;

				numbHolder = allsockets.clientSocket[playerID-1];
				allsockets.clientSocket[playerID-1] =  allsockets.clientSocket[playerID];
				allsockets.clientSocket[playerID] = numbHolder;

				playerID--;
				
			}
		}
	}
	
	//printPlayerData();
}

void sortPlayerList(int client_socket){
	int activePlayers = getPlayerAmount(server_data);
	
for(int i=0;i<4;i++){
	if(allsockets.clientSocket[i]==client_socket){
			while(i<3){
				allsockets.clientSocket[i]=allsockets.clientSocket[i+1];
				strcpy(server_data[i].username, server_data[i+1].username);
				strcpy(playerlist[i].spielername,playerlist[i+1].spielername);
				server_data[i].clientID=server_data[i+1].clientID;
				server_data[i].socket=server_data[i+1].socket;
				server_data[i].currentQuestion=server_data[i+1].currentQuestion;
				server_data[i].currentBitmaks=server_data[i+1].currentBitmaks;

				playerlist[i].score=playerlist[i+1].score;
				playerlist[i].id=playerlist[i+1].id;
				player_ranking[i].score = player_ranking[i+1].score;
				player_ranking[i].rank = player_ranking[i+1].rank;
				playerlist[i+1].score=0;
				playerlist[i+1].id=0;
				player_ranking[i+1].score=0;
				player_ranking[i+1].rank=0;

				playerdata[i].socket = playerdata[i+1].socket;
				playerdata[i+1].socket=0;

				strcpy(spielerliste[i].spielername, spielerliste[i+1].spielername);
				spielerliste[i+1].spielername[0]='\0';
			
				spielerliste[i].id=spielerliste[i+1].id;
				spielerliste[i+1].id=0;
					
				spielerliste[i].score=spielerliste[i+1].score,
				spielerliste[i+1].score=0;

				allsockets.clientSocket[i]=allsockets.clientSocket[i+1];
				allsockets.clientSocket[i+1]=0;
				i++;
			}	
		}
	}
	if(activePlayers==4){
		allsockets.clientSocket[3]=0;
		server_data[3].username[0] ='\0';
		playerlist[3].spielername[0]='\0';
		server_data[3].clientID=0;
		server_data[3].socket=0;
		server_data[3].currentQuestion=0;
		server_data[3].currentBitmaks=0;

		playerlist[3].score=0;
		playerlist[3].id=0;
		player_ranking[3].score=0;
		player_ranking[3].rank=0;
	}
	
}

int createBitmask(int status, char message[], int playerIndex){
	int correctBitmask;	
	int firstNibble;
	if(status){
		firstNibble=0;
		correctBitmask = all_questions[server_data[playerIndex].currentQuestion].bitmask;
			if(message[3]==correctBitmask){
				
				
				unsigned long score = (timeLeft * 1000UL)/all_questions[server_data[playerIndex].currentQuestion].timeLimit;		
				score = 1000-((score+5UL)/10UL)*10UL;
				

				//int score = all_questions[server_data[playerIndex].currentQuestion].timeLimit * 10;
				playerlist[playerIndex].score=playerlist[playerIndex].score+score;
				player_ranking[playerIndex].score=player_ranking[playerIndex].score+score;
				sem_post(&score_semaphore);	
				
				
				
			}
			correctBitmask=correctBitmask+firstNibble;
			return correctBitmask;
	}else{
		firstNibble = 10000000;
		correctBitmask = all_questions[server_data[playerIndex].currentQuestion].bitmask;
		correctBitmask=correctBitmask+firstNibble;
		return correctBitmask;
	}
}

void sendGOV(){
	int activePlayers = getPlayerAmount(server_data);
	for(int i=0;i<activePlayers;i++){
		GOV gov;
		gov = createGOV(i ,playerlist[i].score);
		if(myconfig.send){
			debugHexdump(&gov, sizeof(gov), "S==>C");
		}	
		send(allsockets.clientSocket[i], &gov, sizeof(gov),0);
	}
	
}

void sendQURS(int client_socket, int correctBitmask){
	QURS qurs;
	qurs=createQURS(correctBitmask);
	if(myconfig.send){
		debugHexdump(&qurs, sizeof(qurs), "S==>C");
	}
	send(client_socket, &qurs, sizeof(qurs),0);

}


/*	Die letzte Frage an den Client muss leer sein, dass er weiß, dass der Fragekatalog durch ist. */
void sendLASTQUESTION(int client_socket){
	LQUE lque;
	lque=createLQUE();
	if(myconfig.send){
		debugHexdump(&lque, 3, "S==>C");
	}
	send(client_socket, &lque, 3,0);
}

void sendQUESTION(int client_socket, int currentQuestion, int thisPlayer){
	QUE que;
	que=createQUE(currentQuestion,thisPlayer);
	if(myconfig.send){
		debugHexdump(&que, sizeof(que), "S==>C");
	}
	send(client_socket, &que, sizeof(que),0);
}

void sendCRS(int client_socket){
	for(int i=0;i<catalogs.catamount;i++){
		CRS crs;
		crs = createCRS(catalogs.catalogName[i].catalog);
		if(myconfig.send){
			debugHexdump(&crs, strlen(crs.filename)+3, "S==>C");
		}
	send(client_socket, &crs, strlen(crs.filename)+3,0);
	}
	/* leerer Katalog dass die Clients wissen wann die Liste zu Ende ist.*/
	CRS crs;
	crs = createCRS("");
	if(myconfig.send){
		debugHexdump(&crs, strlen(crs.filename)+3, "S==>C");
	}
	send(client_socket, &crs, strlen(crs.filename)+3,0);

	/* Prüfen ob bereits ein Katalog ausgewäht worden ist */

	if(catalogs.currentCat[0]!='\0'){
		NC nc;
		nc = createNC(catalogs.currentCat);
		if(myconfig.send){
			debugHexdump(&nc, strlen(nc.catname)+3, "S==>C");
		}
		send(client_socket, &nc, strlen(nc.catname)+3,0);
	}
}

void sendCC(char filename[]){
	CC cc;
	cc = createCC(filename);
	if(myconfig.send){
		debugHexdump(&cc, strlen(cc.filename)+3, "S==>C");
	}
	for(int i=0;allsockets.clientSocket[i]!=0 && i < 4;i++){
		if(playerlist[i].id==0){
			continue;
		}
		send(allsockets.clientSocket[i], &cc, sizeof(cc),0);
	}
}

void sendERR(int subtype, char message[], int client_socket){
	ERR err;
	err = createERR(message, subtype);
	if(myconfig.send){
		debugHexdump(&err, strlen(message)+4, "S==>C");
	}
	send(client_socket, &err, strlen(message)+4,0);
}

void sendLST(int activePlayers){
	LST lst;
	lst = createLST(activePlayers);
	if(myconfig.send){ 
		debugHexdump(&lst, ((1+activePlayers)*37)+3, "S==>C");
	}	
	/* Die Sockets in der Struct werden am Anfang alle mit 0 initialisiert und erst gefüllt sobald ein Spieler sich verbindet darum einfach auf 0 prüfen. */
	for(int i=0;allsockets.clientSocket[i]!=0 && i < 4;i++){
		/* Damit nicht auch leere Teile des Playerarrays geschickt werden, werden hier nur so viele Bytes an die jeweiligen Sockets gesendet wie es Spieler gibt
		   Bei 3 Spielern also --> ((1+2)*37)+3 	37 für die Größe der Liste und 1 mal 3 Bytes für 1 Byte Type und 2 Bytes Length */
		send(allsockets.clientSocket[i], &lst, ((1+activePlayers)*37)+3,0);
	}	
}


void sendLOK(int activePlayers, int client_socket){
	LOK lok;
	lok = createLOK(activePlayers);
	if(myconfig.send){
		debugHexdump(&lok, sizeof(lok), "S==>C");
	}
	send(client_socket, &lok, sizeof(lok),0);
		/* Kritischer Abschnitt -- START -- Playerlist*/
		pthread_mutex_lock (&mutex_struct.playerlist_mutex);
		strcpy(playerlist[activePlayers].spielername, server_data[activePlayers].username);
		playerlist[activePlayers].score=0;
		playerlist[activePlayers].id=activePlayers;
		pthread_mutex_unlock (&mutex_struct.playerlist_mutex);
		/* Kritischer Abschnitt -- ENDE -- Playerlist*/
}
