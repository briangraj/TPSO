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


//ESTRUCTURAS
typedef struct {
	char* nombre;
	Function* funcion;
	char* descripcion;
}t_comando;



//COMANDOS
int				com_pausar						(char* parametro);
int				com_continuar					(char* parametro);
int				com_bloquear					(char* parametro);
int				com_desbloquear					(char* parametro);
int				com_listar						(char* parametro);
int				com_kill						(char* parametro);
int				com_status						(char* parametro);
int				com_deadlock					(char* parametro);


//VARIABLES GLOBALES
t_comando comandos[] = {
		{"pausar", com_pausar, "El Planificador no le dará nuevas órdenes de ejecución a ningún ESI mientras se encuentre pausado"},
		{"continuar", com_continuar, "Permite que el planificador de nuevas ordenes de ejecución a los procesos ESI"},
		{"bloquear", com_bloquear, "Se bloqueará el proceso ESI hasta ser desbloqueado, especificado por dicho <ID> en la cola del recurso <clave>"},
		{"desloquear", com_desbloquear, "Se desbloqueara el proceso ESI con el ID especificado. Solo se desbloquearán ESIs que fueron bloqueados con la consola"},
		{"listar", com_listar, "Lista los procesos encolados esperando al recurso"},
		{"kill", com_kill, "Finaliza el proceso"},
		{"status", com_status, "Información sobre las instancias del sistema"},
		{"deadlock", com_deadlock, "Permitirá analizar los deadlocks que existan en el sistema y a que ESI están asociados"},
		{ (char *)NULL, (Function *)NULL, (char *)NULL }
};



//FUNCIONES
void			levantar_consola				(void* param);
char*			stripwhite						(char* string);
int 			ejecutar_linea					(char* linea);
t_comando*		find_command					(char* nombre);






#endif /* FUNCIONES_CONSOLA_H_ */
