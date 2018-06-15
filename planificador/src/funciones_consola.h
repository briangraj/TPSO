/*
 * funciones_consola.h
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#ifndef FUNCIONES_CONSOLA_H_
#define FUNCIONES_CONSOLA_H_


//INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <readline/rltypedefs.h>
#include <readline/chardefs.h>
#include "funciones_planificador.h"
#include <conexiones/estructuras_coord.h>

//ESTRUCTURAS
typedef struct {
	char* nombre;
	Function* funcion;
	char* descripcion;
}t_comando;

typedef struct {
	int id_espera;
	t_list* esis_por_recurso; // tiene una lsita de t_involucrados
} t_espera_circular;

typedef struct{
	int id_esi_duenio;
	int id_bloqueado;
	char* recurso;
} t_involucrados;

//VARIABLES GLOBALES
int SOCKET_COORDINADOR_CONSOLA;
t_list* esperas_circulares;
char* recurso_inicial;
bool encontre_un_ciclo;
t_espera_circular* nueva_espera;

//COMANDOS
int				com_pausar											(char* parametro);
int				com_continuar										(char* parametro);
int				com_bloquear										(char* parametro);
int				com_desbloquear										(char* parametro);
int				com_listar											(char* parametro);
int				com_kill											(char* parametro);
int				com_status											(char* parametro);
int				com_deadlock										(char* parametro);
int 			com_check											(char* parametro);


//VARIABLES GLOBALES
t_comando comandos[10];


//FUNCIONES
void			levantar_consola									(void* param);
void			setear_comandos										();
char*			stripwhite											(char* string);
int 			ejecutar_linea										(char* linea);
t_comando*		find_command										(char* nombre);
void 			imprimir											(char* cadena);
char** 			controlar_y_obtener_parametros						(char* parametro, int cantidad_parametros);
void 			liberar_parametros									(char** parametros, int cantidad_parametros);
void 			imprimir_cola_bloqueados							(char* clave);
t_info_status* 	recibir_info_status									();
void 			mostrar_info_status									(t_info_status* info_status);
void 			imprimir_esperas_circulares							();
void 			esperas_destroyer									(void* elem);
void 			involucrados_destroyer								(void* elem);
void 			cargar_si_recurso_forma_parte_de_un_deadlock		(t_bloqueados_por_clave* bloqueados_por_clave);
t_list* 		asignados_para_el_esi								(int id_esi);
bool 			hay_que_descartarla									(t_bloqueados_por_clave* bloqueados_por_clave);
t_list* 		duplicar_lista_involucrados							(t_list* involucrados);
void 			mostrar_cola_finalizados							();
void 			mostrar_cola_listos									();

#endif /* FUNCIONES_CONSOLA_H_ */
