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

#include "protocolos.h"

int crear_socket();
int conectar_socket_a(char ip[], int puerto, int sockfd);
void bindear_socket(int listener, char ip[], int puerto, t_log* log);
struct sockaddr_in cargar_direccion(char ip[], int puerto);
int conectarse_a_server(char* nombre_cliente, char* nombre_server, char* ip_server, int puerto_server, int socket_server, t_log* log_cliente);

#endif /* SOCKETS_H_ */
