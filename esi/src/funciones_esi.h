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


//ESTRUCTURAS
typedef struct{
	int informe_coordinador;
	char* sentencia_ejecutada;//REVISAR SI ES CHAR* U OTRA COSA
} resultado_ejecucion;


//FUNCIONES
void					iniciar_esi							();
void					crear_log							();
void					leer_archivo_config					();
resultado_ejecucion 	ejecutar_proxima_sentencia			(FILE* script);
int 					informar_resultado_al_usuario		(resultado_ejecucion informe_ejecucion);
bool					verificar_sentencias_restantes		(FILE* script);
void					finalizar							();



#endif /* FUNCIONES_ESI_H_ */
