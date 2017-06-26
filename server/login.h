/*
 * Felisberto, Sensoy & Wünsch
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * login.h: Header für das Login
 */

#ifndef LOGIN_H
#define LOGIN_H
#include "rfc.h"

#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

pthread_t login_thread;
pthread_t score_thread;




typedef struct{
	char username[MAX_NAME];
	int clientID;
	pthread_t user_thread;
	pthread_t score_thread;
	int socket;
	int catAmount;
	int currentQuestion;
	int currentBitmaks;
	int gameOver;
	int finPlayers;
	int gameRunning;
}sd;

int readMessage(char client_message[], int client_socket, sd server_data[]);
int getPlayerAmount(sd server_data[]);
int checkPlayerName(char client_message[], sd server_data[], int activePlayers);
int startThreads();
int lockFileExists();
int removeFile();
int cleanUpSystem();

void druckeFehler();
void setDefaults();
void killHandler();

void printPlayerData();

void *login_handler(void* noParam);

typedef struct __attribute__((packed)){
	int clientSocket[4];
	int server_socket;
}as;

typedef struct __attribute__((packed)){
	char loader[200];
	char catalogs[200];
	int port;
	int monochrome;
	int recieve;
	int send;
	int debug;
}mc;

sd server_data[4];
as allsockets;
mc myconfig;

#endif
