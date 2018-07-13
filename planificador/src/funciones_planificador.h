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
#include <commons/string.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <conexiones/threads.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

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
float ALFA_PLANIFICACION;
char** CLAVES_BLOQUEADAS;
int PLANIFICADOR_PID;

t_algoritmo ALGORITMO_PLANIFICACION;

bool PAUSA;

t_log* log_planif;
t_config* archivo_config;

fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
fd_set master;   // conjunto maestro de descriptores de fichero		//Por comodidad lo pongo aca

t_list* cola_de_listos;
t_list* colas_de_bloqueados;
t_list* cola_finalizados;
t_list* colas_de_asignaciones;

pthread_mutex_t semaforo_pausa;
pthread_mutex_t semaforo_cola_bloqueados;
pthread_mutex_t semaforo_cola_finalizados;
pthread_mutex_t semaforo_cola_listos;
pthread_mutex_t semaforo_asignaciones;
pthread_mutex_t semaforo_flag_bloqueo;

int id_esi_activo;
int proximo_id;

struct{
	bool hay_que_bloquear_esi_activo;
	bool fue_bloqueado_consola;
	char* clave_bloqueo;
}flag_bloqueo;

// Estructuras
typedef struct{
	int ID;
	int socket;
	float ultima_rafaga_real;
	float ultima_estimacion;
	float tiempo_espera;
	int tiempo_total_espera;
	int total_instrucciones_ejecutadas;
	int tiempo_total_bloqueado;
	float estimacion_actual;
}t_ready;

typedef struct{
	t_ready* info_ejecucion;
	bool bloqueado_por_consola;
	bool bloqueado_por_ejecucion;
}t_blocked;

typedef struct{
	int ID;
	int tiempo_total_espera;
	int total_instrucciones_ejecutadas;
	int tiempo_total_bloqueado;
	char* exit_text;
}t_ended;

typedef struct{
	char* clave;
	int id_proximo_esi;
	t_list* bloqueados;
}t_bloqueados_por_clave;

typedef struct{
	int id_esi;
	t_list* recursos_asignados;
}t_recursos_por_esi;

// Funciones
void 						signal_handler								(int sig_num);
void						iniciar_planificador						(int loggear);
void						leer_archivo_config							();
void 						bloquear_claves_config						();
void						aniadir_cliente								(fd_set* master, int cliente, int* fdmax);
void						atender_handshake							(int socket_cliente);
void						atender_protocolo							(int protocolo, int socket_cliente);
void						desconectar_cliente							(int cliente);
int							conectarse_a_coordinador					(int remitente);
void 						aniadir_a_listos							(t_ready esi);
void 						aniadir_a_colas_de_asignaciones				(t_ready nuevo_esi);
t_ready* 					duplicar_esi_ready							(t_ready esi);
void 						planificar									(t_ready* esi_ready);
void 						mandar_a_ejecutar							();
void 						mover_a_finalizados							(t_ready* esi_ejecucion, char* exit_text);
void 						actualizar_esperas							();
void						actualizar_esperas_bloqueados				();
void 						insertar_ordenado							(t_ready* esi_ready);
float 						estimacion									(t_ready* esi_ready);
void 						comparar_desde								(int indice_comparacion, bool (*funcion_comparacion)(void*, void*), t_ready* esi_ready);
bool 						comparar_sjf								(void* un_esi, void* otro_esi);
bool 						comparar_hrrn								(void* un_esi, void* otro_esi);
float						ratio										(t_ready* esi);
void 						ordenar_hrrn								(t_ready* esi_ready);
void 						finalizar									();
void 						clave_destroyer								(void* elemento);
t_ready*					esi_activo									();
void 						asignacion_destroyer						(void* elemento);
t_blocked* 					crear_t_bloqueado							(t_ready* esi_ready);
void 						eliminar_de_bloqueados						(t_ready* esi);
t_blocked* 					proximo_no_bloqueado_por_consola			(t_list* bloqueados);
void 						blocked_destroyer							(void* elem);
void 						actualizar_disponibilidad_recursos			(int id_esi);
int 						intentar_asignar							(int id_esi,char* clave);
void 						atender_get_clave							();
void 						atender_store								();
void 						asignar_recurso_al_esi						(int id_esi, char* recurso);
void 						crear_entrada_bloqueados_del_recurso						(int id_esi, char* recurso);
void 						actualizar_privilegiado						(t_bloqueados_por_clave* bloqueados_de_la_clave);
int 						actualizar_cola_de_bloqueados_para			(int id_esi_que_lo_libero, char* recurso);
t_bloqueados_por_clave* 	encontrar_bloqueados_para_la_clave			(char* recurso);
void 						imprimir_estado_cola_listos					();
bool 						verificar_tenencia_de_la_clave				(int id_esi, char* clave);
void 						atender_set									();
t_ready* 					buscar_en_bloqueados						(int id_esi);
t_ready* 					buscar_en_ready								(int id_esi);
void 						hay_que_bloquear_esi_activo					(char* clave, bool fue_bloqueado_por_consola);
void 						bloquear_esi_activo							(char* clave, bool fue_bloqueado_por_consola);
void 						imprimir_estado_cola_bloqueados				(char* clave);
t_ready* 					encontrar_esi_de_socket						(int socket);
void 						finalizado_destroyer						(void* elem);
void 						free_elem									(void* elemento);
t_list* 					asignados_para_el_esi					(int id_esi);

// Funciones MOCK

void 			ejecutar_mock					(int socket_cliente);

#endif /* FUNCIONES_PLANIFICADOR_H_ */
