/*
 * hilo_instancia.c
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#include "hilo_instancia.h"

void crear_hilo_instancia(t_instancia* instancia){
	CANTIDAD_ENTRADAS = 20;
	TAMANIO_ENTRADA = 100;

	crear_hilo(atender_instancia, (void*) instancia);
}

void crear_instancia(int id, int socket){
	t_instancia* instancia = malloc(sizeof(t_instancia));

	instancia->id = id;
	instancia->pedidos = list_create();
	sem_init(&instancia->sem, 0, 0);
	instancia->socket = socket;
	instancia->esta_conectado = true;
}

int recibir_id(int socket){
	int id;

	if(recv(socket, &id, sizeof(int), MSG_WAITALL) <= 0)
		return -1;

	return id;
}

void* atender_instancia(void* socket_instancia){
	int socket = *((int*)socket_instancia);

	enviar_configuracion_instancia(socket);

	while(true) {

	}

	return NULL;
}

void enviar_configuracion_instancia(int socket_instancia){
	int tamanio_payload = sizeof(CANTIDAD_ENTRADAS) + sizeof(TAMANIO_ENTRADA);
	void* payload = malloc(tamanio_payload);

	memcpy(payload, &CANTIDAD_ENTRADAS, sizeof(CANTIDAD_ENTRADAS));
	memcpy(payload + sizeof(CANTIDAD_ENTRADAS), &TAMANIO_ENTRADA, sizeof(TAMANIO_ENTRADA));

	enviar_paquete(CONFIGURACION_ENTRADAS, socket_instancia, tamanio_payload, payload);
}
