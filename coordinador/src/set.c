/*
 * set.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "set.h"

t_solicitud* crear_set(int socket, int id){
	t_solicitud* solicitud = crear_solicitud(OPERACION_SET, id, socket);

	solicitud->clave = recibir_string(socket);
	solicitud->valor = recibir_string(socket);

	return solicitud;
}

t_mensaje serializar_set_a_instancia(char* clave, char* valor){
	int tam_clave = strlen(clave) + 1;
	int tam_valor = strlen(valor) + 1;

	int tam_payload = sizeof(int) + tam_clave + sizeof(int) + tam_valor;

	t_mensaje mensaje = crear_mensaje(OPERACION_SET, tam_payload);

	serializar_string(mensaje.payload, clave);

	serializar_string(mensaje.payload + sizeof(int) + tam_clave, valor);

	return mensaje;
}

int realizar_set(t_solicitud* solicitud){
	return 0;//TODO mock
}
