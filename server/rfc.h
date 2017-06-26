/*
 * Felisberto, Sensoy & Wünsch
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 *
 * rfc.h: Definitionen für das Netzwerkprotokoll gemäß dem RFC
 */

#ifndef RFC_H
#define RFC_H
#define TRUE 1
#define FALSE 0
#define MAX_NAME 50
#define MAX_PLAYERS 4

#include "common/question.h"
#include "score.h"

#include <pthread.h>
#include <semaphore.h>



enum{
	LRQ_TYPE = 1,
	LOK_TYPE,
	CRQ_TYPE,
	CRE_TYPE,
	CCH_TYPE,
	LST_TYPE,
	STG_TYPE,
	QRQ_TYPE,
	QUE_TYPE,
	QAN_TYPE,
	QRE_TYPE,
	GOV_TYPE,
	ERR_TYPE = 255,

};



typedef struct __attribute__((packed)){
	char spielername[32];
	uint32_t score;
	uint8_t id;
}pl;
pl playerlist[4];

typedef struct __attribute__((packed)){
	char question_text[256];
	char answer_1[128];
	char answer_2[128];
	char answer_3[128];
	char answer_4[128];
	uint8_t timeLimit;
}qd;
qd question_data;





typedef struct __attribute__((packed)){
	uint8_t type;
	uint16_t length;
	uint8_t rfcversion;
	uint8_t clientid;
}LOK;

typedef struct __attribute__((packed)){
	uint8_t type;
	uint16_t length;
	uint8_t filename[50];
}CRS;

typedef struct __attribute__((packed)){
	uint8_t type;
	uint16_t length;
	uint8_t filename[50];
}LCRS;

typedef struct __attribute__((packed)){
	uint8_t type;
	uint16_t length;
	pl playerlist[4];
}LST;

typedef struct __attribute__((packed)){
	uint8_t type;
	uint16_t length;
	uint8_t subtype;
	uint8_t message[200];
}ERR;

typedef struct __attribute__((packed)){
	uint8_t type;
	uint16_t length;
	uint8_t filename[50];
}CC;

typedef struct __attribute__((packed)){
	uint8_t type;
	uint16_t length;
	uint8_t catname[50];
}NC;


typedef struct __attribute__((packed)){
	uint8_t type;
	uint16_t length;
	qd question_data;
}QUE;


typedef struct __attribute__((packed)){
	uint8_t type;
	uint16_t length;
	qd question_data;
}LQUE;


typedef struct __attribute__((packed)){
	uint8_t type;
	uint16_t length;
	uint8_t answerData;
}QUR;

typedef struct __attribute__((packed)){
	uint8_t type;
	uint16_t length;
	uint8_t rank;
	uint32_t score;
}GOV;


typedef struct __attribute__((packed)){
	uint8_t type;
	uint16_t length;
	uint8_t bitMask;
}QURS;



typedef struct{
	pthread_mutex_t playerlist_mutex;
	pthread_mutex_t socket_mutex;
	pthread_mutex_t sem_mutex;
	int sem;
}ms;
ms mutex_struct;

typedef struct __attribute__((packed)){
	char spielername[32];
	uint32_t score;
	uint8_t id;
}liste;
liste spielerliste[4];





LQUE createLQUE();
QURS createQURS(int correctBitmask);
QUE createQUE(int currentQuestion, int currentPlayer);
LOK createLOK(int id);
CRS createCRS(char catname[]);
LCRS createLastCRS();
LST createLST(int activePlayers);

ERR createERR(char message[], int subtype);
CC createCC(char filename[]);
NC createNC(char catname[]);
QUR createQUR(int lastByte);
GOV createGOV(int rank, int score);
#endif

