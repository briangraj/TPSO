/*
 * hilo_instancia.c
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#include "hilo_instancia.h"

void crear_hilo_instancia(int socket_instancia){
	crear_hilo(atender_instancia, (void*) socket_instancia);
}

void* atender_instancia(void* socket_instancia){
	enviar_configuracion_instancia(*((int*)socket_instancia));

	return NULL;
}

void enviar_configuracion_instancia(int socket_instancia){
	int tamanio_payload = sizeof(CANTIDAD_ENTRADAS) + sizeof(TAMANIO_ENTRADA);
	void* payload = malloc(tamanio_payload);

	memcpy(payload, &CANTIDAD_ENTRADAS, sizeof(CANTIDAD_ENTRADAS));
	memcpy(payload + sizeof(CANTIDAD_ENTRADAS), &TAMANIO_ENTRADA, sizeof(TAMANIO_ENTRADA));

	enviar_paquete(CONFIGURACION_ENTRADAS, socket_instancia, tamanio_payload, payload);
}
