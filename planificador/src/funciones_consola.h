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
t_comando comandos[9];



//FUNCIONES
void			levantar_consola				(void* param);
void			setear_comandos					();
char*			stripwhite						(char* string);
int 			ejecutar_linea					(char* linea);
t_comando*		find_command					(char* nombre);






#endif /* FUNCIONES_CONSOLA_H_ */
