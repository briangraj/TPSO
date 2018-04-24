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
#include <commons/collections/list.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <conexiones/threads.h>

typedef enum{
	SJF_SD,
	SJF_CD,
	HRRN
}t_algoritmo;

char* IP_PLANIFICADOR;
char* IP_COORDINADOR;
int PUERTO_PLANIFICADOR;
int PUERTO_COORDINADOR;
int SOCKET_COORDINADOR;
int ESTIMACION_INICIAL;
int ALFA_PLANIFICACION;
t_algoritmo ALGORITMO_PLANIFICACION;

t_log* log_planif;
t_config* archivo_config;

fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
fd_set master;   // conjunto maestro de descriptores de fichero		//Por comodidad lo pongo aca

t_list* cola_de_listos;
t_list* colas_de_bloqueados;
t_list* cola_finalizados;

int proximo_id;

// Estructuras
typedef struct{
	int ID;
	int socket;
	float ultima_rafaga_real;
	float ultima_estimacion;
	float tiempo_espera;
	float estimacion_actual;
}t_ready;

typedef struct{

}t_blocked;

typedef struct{

}t_ended;

typedef struct{
	char* clave;
	t_list* bloqueados;
}t_bloqueados_por_clave;

// Funciones
void			iniciar_planificador			(int loggear);
void			leer_archivo_config				();
void			aniadir_cliente					(fd_set* master, int cliente, int* fdmax);
void			atender_handshake				(int socket_cliente);
void			atender_protocolo				(int protocolo, int socket_cliente);
void			desconectar_cliente				(int cliente);
int				conectarse_a_coordinador		();
void 			aniadir_a_listos				(t_ready esi);
t_ready* 		duplicar_esi_ready				(t_ready esi);
void 			planificar						(t_ready* esi_ready);
void 			mandar_a_ejecutar				();
void 			insertar_ordenado				(t_ready* esi_ready);
float 			estimacion						(t_ready* esi_ready);
void 			comparar_desde					(int indice_comparacion, bool (*funcion_comparacion)(void*, void*), t_ready* esi_ready);
bool 			comparar_sjf					(void* un_esi, void* otro_esi);
bool 			comparar_hrrn					(void* un_esi, void* otro_esi);
float			ratio							(t_ready* esi);
void 			ordenar_hrrn					(t_ready* esi_ready);
void 			finalizar						();
void 			clave_destroyer					(void* elemento);

// Funciones MOCK

void 			ejecutar_mock					(int socket_cliente);

#endif /* FUNCIONES_PLANIFICADOR_H_ */
