/*
 * funciones_esi.h
 *
 *  Created on: 18 abr. 2018
 *      Author: utnso
 */

#ifndef FUNCIONES_ESI_H_
#define FUNCIONES_ESI_H_

#include <commons/config.h>
#include <conexiones/protocolos.h>
#include <conexiones/sockets.h>
#include <conexiones/serializacion.h>
#include <commons/log.h>

//VARIABLES GLOBALES
char* IP_PLANIFICADOR;
char* IP_COORDINADOR;
int PUERTO_PLANIFICADOR;
int PUERTO_COORDINADOR;

int SOCKET_PLANIFICADOR;
int SOCKET_COORDINADOR;

t_log* log_esi;
t_config* archivo_config;

#endif /* FUNCIONES_ESI_H_ */
