/*
 * Felisberto, Sensoy & Wünsch
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * clientthread.h: Header für den Client-Thread
 */

#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

void *connection_handler(void* socket_desc);
void encodeMessage(char message[], int client_socket);
void sendCC(char filename[]);
void sendCRS(int client_socket);
void sendLastCRS(int client_socket);
void sendQUE(int client_socket);
void sendQUR(int client_socket, int lastByte);
void sendLastQUE(int client_socket);

#endif
