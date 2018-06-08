/*
 * t_solicitud.c
 *
 *  Created on: 6 jun. 2018
 *      Author: utnso
 */
#include "t_solicitud.h"

t_solicitud* crear_solicitud(int instruccion, int id){
	t_solicitud* solicitud = malloc(sizeof(t_solicitud));

	solicitud->instruccion = instruccion;
	solicitud->id_esi = id;
	sem_init(&solicitud->solicitud_finalizada, 0, 0);
	solicitud->respuesta_a_esi = NULL;
	solicitud->resultado_instancia = NULL;

	return solicitud;
}
