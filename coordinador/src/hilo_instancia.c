/*
 * hilo_instancia.c
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#include "hilo_instancia.h"

void crear_hilo_instancia(t_instancia* instancia){
	crear_hilo(atender_instancia, (void*) instancia);
}

void* crear_instancia(int id, int socket){
	t_instancia* instancia = malloc(sizeof(t_instancia));

	instancia->id = id;
	instancia->pedidos = queue_create();
	sem_init(&instancia->sem, 0, 0);
	instancia->socket = socket;
	instancia->esta_activa = true;

	return instancia;
}

int recibir_claves(t_instancia* instancia){
	int cant_claves;

	if(recv(instancia->socket, &cant_claves, sizeof(int), MSG_WAITALL) <= 0){
		log_error(LOG_COORD, "no se pudo recibir la cantidad de claves de la instancia %d", instancia->id);
		return -1;
	}

	int i;
	for(i = 0; i < cant_claves; i++){
		char* clave = recibir_string(instancia->socket);
		agregar_clave(instancia, clave);
	}

	return 0;
}

void* atender_instancia(void* instancia_void){
	t_instancia* instancia = (t_instancia*)instancia_void;
	t_solicitud* pedido;

	if(recibir_claves(instancia) == -1){
		log_error(LOG_COORD, "no se pudieron recibir las claves de la instancia %d", instancia->id);
		return -1;
	}

	if(enviar_config_instancia(instancia->socket) == -1){
		log_error(LOG_COORD, "no se pudo enviar la config a la instancia %d", instancia->id);
		return -1;
	}

	while(true) {
		sem_wait(&instancia->sem);
		pedido = (t_solicitud*)queue_pop(instancia->pedidos);

		if(enviar_pedido(pedido, instancia->socket) == -1){
			log_error(
					LOG_COORD,
					"no se pudo enviar el pedido de la instruccion %d a la instancia %d",
					pedido->instruccion,
					instancia->id
			);

			return -1;
		}

		//recv(resultado_pedido)
	}

	return 0;
}

int enviar_config_instancia(int socket_instancia){
	int tamanio_payload = sizeof(CANTIDAD_ENTRADAS_TOTALES) + sizeof(TAMANIO_ENTRADA);
	void* payload = malloc(tamanio_payload);

	memcpy(payload, &CANTIDAD_ENTRADAS_TOTALES, sizeof(CANTIDAD_ENTRADAS_TOTALES));
	memcpy(payload + sizeof(CANTIDAD_ENTRADAS_TOTALES), &TAMANIO_ENTRADA, sizeof(TAMANIO_ENTRADA));

	return enviar_paquete(CONFIGURACION_ENTRADAS, socket_instancia, tamanio_payload, payload);
}

int enviar_pedido(t_solicitud* solicitud, int socket){
	t_mensaje mensaje;

	switch(solicitud->instruccion){
	case OPERACION_GET:{
		mensaje = serializar_get(solicitud->clave);
		break;
	}
	case OPERACION_SET:{
		mensaje = serializar_set(solicitud->clave, solicitud->valor);
		break;
	}
	case OPERACION_STORE:{
		mensaje = serializar_store(solicitud->clave);
		break;
	}
	default:
		return -1;
	}

	return enviar_mensaje(mensaje, socket);
}

