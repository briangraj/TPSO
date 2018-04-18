/*
 * cliente.h
 *
 *  Created on: 20/9/2017
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <commons/log.h>

int crear_socket();
int conectar_socket_a(char ip[], int puerto, int sockfd);
void bindear_socket(int listener, char ip[], int puerto, t_log* log);
struct sockaddr_in cargar_direccion(char ip[], int puerto);

#endif /* SOCKETS_H_ */
