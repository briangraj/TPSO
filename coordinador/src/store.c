/*
 * store.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "store.h"

t_solicitud* crear_store(int socket){
	t_solicitud* solicitud = malloc(sizeof(t_solicitud));

	solicitud->instruccion = OPERACION_GET;
	solicitud->clave = recibir_string(socket);

	return solicitud;
}
