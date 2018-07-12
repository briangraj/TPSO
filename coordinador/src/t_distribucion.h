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
	t_instancia* (*algoritmo)(t_solicitud*);
	t_list* rangos;
} t_distribucion;

typedef struct {
	char inicio;
	char fin;
	int id_instancia;
}t_rango;

t_instancia* equitative_load				(t_solicitud* solicitud);
t_instancia* least_space_used				(t_solicitud* solicitud);
t_instancia* key_explicit					(t_solicitud* solicitud);
void		 set_rangos						();
t_instancia* elegir_instancia_segun_rango	(char* clave);
int 		 ceiling						(double numero);

#endif /* T_DISTRIBUCION_H_ */