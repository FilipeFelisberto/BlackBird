/*
 * Felisberto, Sensoy & Wünsch
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * catalog.c: Implementierung der Fragekatalog-Behandlung und Loader-Steuerung
 */

#include "common/util.h"
#include "common/question.h"
#include "common/server_loader_protocol.h"
#include "rfc.h"
#include "common/question.h"
#include "catalog.h"
  
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define QUESTION_LENGTH 770

/*	Funktionen die im File aufgerufen werden:
 *
 *	startLoader()	->	Startet den Loader, als Parameter entweder der Default-Pfad der Kataloge oder der angegebene Parameter
 * 			->	liegt in diesem File
 *		
 *	browseCats()	->	Lädt alle Dateien die sich im angegebenen Verzeichnis befinden in die Katalog-Struktur. 
 * 			->	liegt in diesem File
 *
 *	loadCat()	->	Lädt den angegebenen Katalog in den Shared-Memory, als Parameter der Name des katalogs.
 * 			->	liegt in diesem File
 *	
 *
*/

int stdoutPipe[2];
int stdinPipe[2];
int questionAmount;
int fd;
Question *shmData;
int shmLen;

int startLoader(char catpath[]){
     
    /* Pipes erzeugen */
    if(pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1){
      errorPrint("Pipe konnte nicht erzeugt werden");
      return 3;
    }
    /* Kindprozess abspalten */
    int forkResult = fork();
    if( forkResult < 0 )         {
        errorPrint("Kindprozess konnte nicht erzeugt werden");
        return 0;
    }
     
    else if( forkResult == 0 )     /* im Kindprozess */
    {
        if( dup2(stdinPipe[0], STDIN_FILENO) == -1)
        {
            errorPrint("[cat] dup2(stdinPipe[0], STDIN_FILENO)");
            return 0;
        }
  
    /* Umleitung der Standardausgabe */
    /* dup2(alte Standardausgabe, neue Standardausgabe(pipe))*/
        if(dup2(stdoutPipe[1], STDOUT_FILENO) == -1)
        {
            errorPrint("[cat] dup2(stdoutPipe[1], STDOUT_FILENO)");
            return 0;
        }
         
        /*** Schließen aller Pipe-Deskriptoren.
    *   Nach dem exec kennt der Kindprozess diese nicht mehr und spricht
    *   die Pipes selbst über stdin und stdout an. */
    close(stdinPipe[0]); 
    close(stdinPipe[1]);
    close(stdoutPipe[0]); 
    close(stdoutPipe[1]);
         
    /* Anderes Programm in die vorbereitete Prozessumgebung laden */
        int exeResult = execl("loader", "loader",  catpath, NULL);
    if( exeResult < 0 )
        {
            errorPrint("[cat] Konnte Loader nicht starten. \n");
            perror("[cat] execl");
            return 0;
        }
        if (exeResult == 0){
        errorPrint("[cat] Loader wurde gestartet");
    }
 
    }
    else                /* im Elternprozess */
    {
      /*** Schließen der hier nicht benötigten Enden der Pipes,
      *   also stdinPipe[0] und stdoutPipe[1] */
      close(stdinPipe[0]); 
      close(stdoutPipe[1]);
    }  
infoPrint("Der Loader wurde erfolgreich gestartet.");
return 1;
}


void loadCat(char catName[]){
	char message[100];
	int firstDez;
	int secondDez;
	
	//shm_unlink("/quiz_reference_implementation");
	//fd=shm_open("quiz_reference_implementation", O_RDWR, (mode_t)0);
	//shmData = mmap(NULL, shmLen, PROT_READ, MAP_SHARED,fd, 0);
	
	write(stdinPipe[1], "LOAD_CMD_PREFIX" , 4);
	write(stdinPipe[1], " " , 1);
	write(stdinPipe[1], catName , strlen(catName));
	write(stdinPipe[1], "\n" , 1);
	read(stdoutPipe[0],message, sizeof(message)); 
	if(message[13]=='='){
		if(		((message[15]-48) > -1 && (message[15]-48 <10)) && ((message[16]-48) > -1 && message[16]-48 < 10)){
				firstDez = message[15]-48;
				secondDez = message[16]-48;
				questionAmount = (firstDez*10) + secondDez; 	
				catalogs.questAmount=questionAmount;
		}else{	
			questionAmount=message[15]-48;
			catalogs.questAmount=questionAmount;
		}
	}
	shmLen=questionAmount*QUESTION_LENGTH;


	fd=shm_open("quiz_reference_implementation", O_RDWR, 0600);
	shmData = mmap(NULL, shmLen, PROT_READ, MAP_SHARED,fd, 0);

	for(int i=0;i<questionAmount;i++){
		strcpy(all_questions[i].question_text, shmData[i].question);
		strcpy(all_questions[i].answer_1, shmData[i].answers[0]);
		strcpy(all_questions[i].answer_2, shmData[i].answers[1]);
		strcpy(all_questions[i].answer_3, shmData[i].answers[2]);
		strcpy(all_questions[i].answer_4, shmData[i].answers[3]);
		all_questions[i].timeLimit=shmData[i].timeout;
		all_questions[i].bitmask=shmData[i].correct;
	}
}


int browseCats(){

char message[100];
    char command[] = "BROWSE\n";
	
    int ascii_enter='\n';
    int termJ;
    int length=0;

    int catCounter=0;
    int innerLoop=0;

    write(stdinPipe[1], BROWSE_CMD , sizeof(BROWSE_CMD));
    write(stdinPipe[1], "\n" , 1);
    read(stdoutPipe[0],message, sizeof(message)); 

	/* Hier werden die geladenen Kataloge in die Struktur gespeichert, ascii_enter entspricht einem '\n' um zu prüfen wann ein name vorbei ist. */

    	for(int i=0;message[i]!='\0';i++){
		length++;
		if(message[i]==ascii_enter){	
			innerLoop=length-1;		
			for(int j=0;j<length;j++){
				catalogs.catalogName[catCounter].catalog[j]=message[i-innerLoop];	
				innerLoop--;
				termJ=j;
			}
			catalogs.catalogName[catCounter].catalog[termJ]='\0';
			length=0;
			catCounter++;
			catalogs.catamount=catCounter+1;
			length=0;
		}	
  	}
	if(catalogs.catamount==1){
			infoPrint("Folgender Katalog wird gespielt:");
		}
		if(catalogs.catamount>1){
			infoPrint("Folgende Kataloge werden gespielt:");
		}
		if(catalogs.catamount<1){
			errorPrint("Keine Kataloge gefunden!");
			return -1;
		}
		for(int i=0;i<catalogs.catamount;i++){
			if(catalogs.catalogName[i].catalog[0]=='\0'){
				break;
			}else{
				infoPrint("%s" , catalogs.catalogName[i].catalog);
			}
    		}
	return 1;
}

int removeSharedMemory(){
	if(shm_unlink("/quiz_reference_implementation")<0){
		return -1;
	}else{
		return 0;
	}
}

int closeSharedMemoryFD(){
	if(close(fd)<0){
		return -1;
	}else{
		return 0;
	}
	
}

int closeAllPipes(){
	if(close(stdinPipe[1]) < 0  || close(stdoutPipe[0]) <0 ){
			return -1;
	}else{
			return 0;
	}	
}


    
