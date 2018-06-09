/*
 * t_solicitud.c
 *
 *  Created on: 6 jun. 2018
 *      Author: utnso
 */
#include "t_solicitud.h"

t_solicitud* crear_solicitud(int instruccion, int id, int socket){
	t_solicitud* solicitud = malloc(sizeof(t_solicitud));

	solicitud->instruccion = instruccion;
	solicitud->id_esi = id;
	solicitud->socket_esi = socket;
	sem_init(&solicitud->solicitud_finalizada, 0, 0);
	solicitud->respuesta_a_esi = NULL;
	solicitud->resultado_instancia = NULL;

	return solicitud;
}

void setear_error_comunicacion_instancia(t_solicitud* solicitud){
	solicitud->resultado_instancia = ERROR_DE_COMUNICACION;
}

void setear_error_clave_inaccesible(t_solicitud* solicitud){
	solicitud->respuesta_a_esi = ERROR_CLAVE_INACCESIBLE;
}

void setear_operacion_exitosa_instancia(t_solicitud* solicitud){
	solicitud->resultado_instancia = OPERACION_EXITOSA;
}
