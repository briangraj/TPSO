/*
 * funciones_planificador.h
 *
 *  Created on: 19 abr. 2018
 *      Author: utnso
 */

#ifndef FUNCIONES_PLANIFICADOR_H_
#define FUNCIONES_PLANIFICADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <conexiones/protocolos.h>
#include <conexiones/serializacion.h>
#include <conexiones/sockets.h>
#include <commons/log.h>
#include <commons/config.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

char* IP_PLANIFICADOR;
char* IP_COORDINADOR;
int PUERTO_PLANIFICADOR;
int PUERTO_COORDINADOR;
int SOCKET_COORDINADOR;

t_log* log_planif;
t_config* archivo_config;

fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
fd_set master;   // conjunto maestro de descriptores de fichero		//Por comodidad lo pongo aca

// Funciones
void iniciar_planificador();
void leer_archivo_config();
void conectarse_a_coordinador(int socket);
void aniadir_cliente(fd_set* master, int cliente, int* fdmax);
void atender_handshake(int socket_cliente);
void atender_protocolo(int protocolo, int socket_cliente);
void desconectar_cliente(int cliente);

#endif /* FUNCIONES_PLANIFICADOR_H_ */
