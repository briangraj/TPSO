/*
 * set.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "set.h"

t_solicitud* crear_set(int socket){
	t_solicitud* solicitud = malloc(sizeof(t_solicitud));

	solicitud->instruccion = OPERACION_SET;
	solicitud->clave = recibir_string(socket);
	solicitud->valor = recibir_string(socket);

	return solicitud;
}

int realizar_set(t_solicitud* solicitud){

	return 0;
}
