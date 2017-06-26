/*
 * Felisberto, Sensoy & Wünsch
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * user.h: Header für die User-Verwaltung
 */

#ifndef USER_H
#define USER_H
#include "login.h"

/* ... */

#endif




void sendGOV();

void sendCRS(int client_socket);
void sendLastCRS(int client_socket);
void sendCC(char filename[]);
void sendLOK(int activePlayers, int client_socket);
void sendLST(int activePlayers);
void sendERR(int subtype, char message[], int client_socket);

void closeAllSockets();
void sortPlayerList(int client_socket);
void sortPlayerRanks(sd server_data[]);
void encodeMessage(char message[], int client_socket);
int isGameLeader(int client_socket);
int createBitmask(int status, char message[], int playerIndex);
int startTimer();
int startTimer(int client_socket, int playerIndex);

void sendQURS(int client_socket, int correctBitmask);

void sendQUESTION(int client_socket ,int currentQuestion, int thisPlayer);
void sendLASTQUESTION(int client_socket);


typedef struct{
	int socket;
}pd;
pd playerdata[4];
