/*
 * Felisberto, Sensoy & Wünsch
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * main.c: Hauptprogramm des Servers
 */

#include "common/util.h"
#include "clientthread.h"
#include "catalog.h"
#include "rfc.h"
#include "score.h"
#include "login.h"

#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>




/*	Funktionen die im File aufgerufen werden:				
 *
 *	startLoader()	->	Startet den Loader, als Parameter entweder der Default-Pfad der Kataloge oder der angegebene Parameter
 *			-> 	liegt in catalog.h
 *	browseCats()	->	Lädt alle Dateien die sich im angegebenen Verzeichnis befinden in die Katalog-Struktur. 
 *			-> 	liegt in catalog.h
 *	startThreads()	->	Startet den Login-Thread und den Score-Thread.
 *			->	Der Score-Thread wird in scorce.c verarbeitet
 *			->	Der Login-Thread wird in login.c verarbeitet. 
 *			-> 	liegt in login.c
*/

int main(int argc, char **argv)
{
	setProgName(argv[0]);
	setDefaults();
	
	int opt;
	while ((opt = getopt(argc, argv, "c:l:p:dmrs")) != -1) {
		switch(opt){
				case 'c': 	memcpy(myconfig.catalogs,optarg,strlen(optarg)); 
			  			break;
				case 'l': 	memcpy(myconfig.loader,optarg,strlen(optarg)); 
			  			break;
				case 'p': 	myconfig.port=atoi(optarg);
					  	break;  
				case 'd': 	myconfig.debug=TRUE;
					  	debugEnable();
					  	break;	  
				case 'm': 	myconfig.monochrome=TRUE;
					  	styleEnable();
					  	break;		  
				case 'r': 	myconfig.recieve=TRUE;
					  	break;	  
				case 's': 	myconfig.send=TRUE;
					  	break;	
				default:  	druckeFehler();
					  	return -1;	  
	    		}    
	}	
	if(strlen(myconfig.catalogs)==0 || myconfig.port==0){		
	    	druckeFehler(); 
	    	return -1;
	}	
	
	if(lockFileExists()){
		errorPrint("Dieses Programm läuft bereits, das Programm wird beendet.");
		return -1;
	}
	
	if(!startThreads()){
		errorPrint("Fehler beim Erstellen der Threads, das Spiel wird beendet.");
		return -1;
	}
	
	if(!startLoader(myconfig.catalogs)){
		errorPrint("Fehler beim Starten des Loaders, das Spiel wird beendet.");
		return -1;
	}	
	
	if(!browseCats()){
		errorPrint("Fehler beim Browsen der Kataloge, das Spiel wird beendet.");
		return -1;
	}
	
	
	
	pthread_join(score_thread, NULL);
	
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
