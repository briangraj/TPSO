/*
 * distribucion.h
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#ifndef T_DISTRIBUCION_H_
#define T_DISTRIBUCION_H_

#include "coordinador.h"
#include "conexiones/estructuras_coord.h"
#include <math.h>

typedef struct distr{
	int proxima_instancia;
	t_instancia* (*algoritmo)(char*);
	t_list* rangos;
} t_distribucion;

typedef struct {
	char inicio;
	char fin;
	int id_instancia;
}t_rango;

t_instancia* equitative_load				(char* clave);
t_instancia* least_space_used				(char* clave);
t_instancia* key_explicit					(char* clave);
void		 set_rangos						();
t_instancia* elegir_instancia_segun_rango	(char* clave);
int 		 ceiling						(int n, int v);

#endif /* T_DISTRIBUCION_H_ */
