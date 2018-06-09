/*
 * store.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "store.h"

t_solicitud* crear_store(int socket, int id){
	t_solicitud* solicitud = crear_solicitud(OPERACION_STORE, id);

	solicitud->clave = recibir_string(socket);

	return solicitud;
}

t_mensaje serializar_store_a_instancia(char* clave){
	int tam_clave = strlen(clave) + 1;

	t_mensaje mensaje = crear_mensaje(OPERACION_STORE, sizeof(int) + tam_clave);

	memcpy(mensaje.payload, &tam_clave, sizeof(int));

	serializar_string(mensaje.payload + sizeof(int), clave);

	return mensaje;
}

int realizar_store(t_solicitud* solicitud){
	return 0;//TODO mock
}
