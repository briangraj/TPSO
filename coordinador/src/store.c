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

t_mensaje serializar_store(char* clave){
	t_mensaje mensaje;

	mensaje.header = OPERACION_STORE;

	int tam_clave = strlen(clave) + 1;

	mensaje.tam_payload = sizeof(int) + tam_clave;

	mensaje.payload = malloc(mensaje.tam_payload);

	void* aux = mensaje.payload;

	memcpy(aux, &tam_clave, sizeof(int));
	aux += sizeof(int);

	serializar_string(aux, clave);

	return mensaje;
}
