/*
 * funciones_esi.h
 *
 *  Created on: 18 abr. 2018
 *      Author: utnso
 */

#ifndef FUNCIONES_ESI_H_
#define FUNCIONES_ESI_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <conexiones/protocolos.h>
#include <conexiones/sockets.h>
#include <conexiones/serializacion.h>
#include <commons/log.h>
#include <parsi/parser.h>

//VARIABLES GLOBALES
char* IP_PLANIFICADOR;
char* IP_COORDINADOR;
int PUERTO_PLANIFICADOR;
int PUERTO_COORDINADOR;

int SOCKET_PLANIFICADOR;
int SOCKET_COORDINADOR;
int MI_ID;

t_log* log_esi;
t_config* archivo_config;


//ESTRUCTURAS
typedef struct{
	int informe_coordinador;
	char* sentencia_ejecutada;//REVISAR SI ES CHAR* U OTRA COSA
} t_resultado_ejecucion;


//FUNCIONES
void					iniciar_esi							();
void					crear_log							();
void					leer_archivo_config					();
t_resultado_ejecucion 	ejecutar_proxima_sentencia			(FILE* script);
int 					informar_resultado_al_usuario		(t_resultado_ejecucion informe_ejecucion, FILE* script);
void					rollbackear_ultima_sentencia		(char* sentencia_ejecutada, FILE* script);
bool					verificar_sentencias_restantes		(FILE* script);
void					finalizar							();
int 					operacion_get_al_coordinador		(char * clave);
int 					operacion_set_al_coordinador		(char * clave, char* valor);
int 					operacion_store_al_coordinador		(char * clave);




#endif /* FUNCIONES_ESI_H_ */
