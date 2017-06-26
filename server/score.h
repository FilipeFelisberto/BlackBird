/*
 * Felisberto, Sensoy & Wünsch
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * score.h: Header für den Score Agent
 */

#ifndef SCORE_H
#define SCORE_H
#include "login.h"
#include "rfc.h"

#include <semaphore.h>
#include <pthread.h>

typedef struct __attribute__((packed)){
	char question_text[256];
	char answer_1[128];
	char answer_2[128];
	char answer_3[128];
	char answer_4[128];
	uint8_t timeLimit;
	uint8_t bitmask;
}aq;
aq all_questions[50];

typedef struct{
	int score;
	int rank;
}pr;
pr player_ranking[4];

typedef struct{
	int client_socket;
	int score;
}score_struct;
score_struct score_s;


void *score_handler(void* socket_desc);
void sortPlayers();
void createNewList();
void sendNewLST(int spielerAnzahl);
void setRanks();

sem_t score_semaphore;
#endif
