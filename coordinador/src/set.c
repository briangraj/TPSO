/*
 * set.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "set.h"

t_solicitud* crear_set(int socket, int id){
	t_solicitud* solicitud = crear_solicitud(id);

	solicitud->instruccion = OPERACION_SET;
	solicitud->clave = recibir_string(socket);
	solicitud->valor = recibir_string(socket);

	return solicitud;
}

t_mensaje serializar_set_a_instancia(char* clave, char* valor){
	t_mensaje mensaje;

	mensaje.header = OPERACION_SET;

	int tam_clave = strlen(clave) + 1;
	int tam_valor = strlen(valor) + 1;

	mensaje.tam_payload = sizeof(int) + tam_clave + sizeof(int) + tam_valor;

	mensaje.payload = malloc(mensaje.tam_payload);

	void* aux = mensaje.payload;

	serializar_string(aux, clave);
	aux += sizeof(int) + tam_clave;

	serializar_string(aux, valor);

	return mensaje;
}

int realizar_set(t_solicitud* solicitud){
	return 0;//TODO mock
}
