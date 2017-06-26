/*
 * Felisberto, Sensoy & Wünsch
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 *
 * rfc.c: Implementierung der Funktionen zum Senden und Empfangen von
 * Datenpaketen gemäß dem RFC
 */
#include "rfc.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "score.h"



LOK createLOK(int id){
	LOK lok;
	lok.type=LOK_TYPE;
	lok.length=htons(2);
	lok.rfcversion=8;
	lok.clientid=id;
	return lok;
}

QURS createQURS(int correctBitmask){
	QURS qurs;
	qurs.type=QRE_TYPE;
	qurs.length=htons(1);
	qurs.bitMask=correctBitmask;
	return qurs;
}

QUE createQUE(int currentQuestion, int currentPlayer){
	QUE que;
	que.type = QUE_TYPE;
	que.length = htons(769);
	strcpy(que.question_data.question_text, all_questions[currentQuestion].question_text);
	strcpy(que.question_data.answer_1, all_questions[currentQuestion].answer_1);
	strcpy(que.question_data.answer_2, all_questions[currentQuestion].answer_2);
	strcpy(que.question_data.answer_3, all_questions[currentQuestion].answer_3);
	strcpy(que.question_data.answer_4, all_questions[currentQuestion].answer_4);
	que.question_data.timeLimit = all_questions[currentQuestion].timeLimit;
	return que;
}

LQUE createLQUE(){
	LQUE lque;
	lque.type = QUE_TYPE;
	lque.length=htons(0);
	return lque;
}


QUR createQUR(int lastByte){
	QUR qur;
	qur.type=QRE_TYPE;
	qur.length=htons(1);
	qur.answerData = lastByte;
	return qur;
}

GOV createGOV(int rank, int score){
	GOV gov;
	gov.type=GOV_TYPE;
	gov.length=htons(5);
	gov.rank=rank+1;
	gov.score=htonl(score);
	return gov;
}

CC createCC(char filename[]){
	CC cc;
	cc.type=CCH_TYPE;
	cc.length=htons(strlen(filename));
	strcpy(cc.filename, filename);
	return cc;
}

NC createNC(char catname[]){
	NC nc;
	nc.type=CCH_TYPE;
	nc.length=htons(strlen(catname));
	strcpy(nc.catname , catname);
	return nc;
}

CRS createCRS(char catname[]){
	CRS crs;
	crs.type=CRE_TYPE;
	strcpy(crs.filename, catname);
	crs.length=htons(strlen(crs.filename));
	return crs;
}

LCRS createLastCRS(){
	LCRS lcrs;
	lcrs.type=CRE_TYPE;
	strcpy(lcrs.filename, "");
	lcrs.length=htons(strlen(lcrs.filename));
	return lcrs;
}

ERR createERR(char message[], int subtype){
	ERR err;
	err.type=ERR_TYPE;
	err.subtype=subtype;
	err.length=htons(strlen(message)+1);
	strcpy(err.message, message);
	err.message[strlen(message)]='\0';	

	return err;
}


LST createLST(int activePlayers){
	LST lst;
	lst.type=LST_TYPE;
	lst.length=htons((activePlayers+1)*37);
	for(int i=0;i<MAX_PLAYERS;i++){
		memset(lst.playerlist[i].spielername, 0 , sizeof(lst.playerlist[i].spielername));
	}
	for(int i=0;i<MAX_PLAYERS;i++){
		strcpy(lst.playerlist[i].spielername, playerlist[i].spielername);
		lst.playerlist[i].score = htonl(playerlist[i].score);
		lst.playerlist[i].id = playerlist[i].id;
	}

	return lst;
}

