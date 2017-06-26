/*
 * Felisberto, Sensoy & Wünsch
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * catalog.h: Header für die Katalogbehandlung und Loader-Steuerung
 */
 
#ifndef CATALOG_H
#define CATALOG_H
#include "rfc.h"
 
int startLoader(char catalogs[]);
int browseCats();
int removeSharedMemory();
int closeSharedMemoryFD();
int closeAllPipes();

void loadCat(char catName[]);

typedef struct __attribute__((packed)){
	char catalog[50];
}cn;

typedef struct __attribute__((packed)){
	cn catalogName[5];
	int catamount;	
	char currentCat[100];
	int questAmount;
}ct;

cn catName;
ct catalogs;

#endif
