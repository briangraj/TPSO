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

void* crear_instancia(int id, int socket){
	t_instancia* instancia = malloc(sizeof(t_instancia));

	instancia->id = id;
	instancia->pedidos = list_create();
	sem_init(&instancia->sem, 0, 0);
	instancia->socket = socket;
	instancia->esta_conectado = true;

	return instancia;
}

int recibir_id(int socket){
	int id;

	if(recv(socket, &id, sizeof(int), MSG_WAITALL) <= 0)
		return -1;

	return id;
}

void* atender_instancia(void* instancia_void){
	t_instancia* instancia = (t_instancia*)instancia_void;
	t_solicitud* pedido;
	enviar_configuracion_instancia(instancia->socket);

	while(true) {
		sem_wait(&instancia->sem);
		pedido = (t_solicitud*)list_remove(instancia->pedidos, 0);

		enviar_pedido(pedido, instancia->socket);
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

int enviar_pedido(t_solicitud* solicitud, int socket){
	int tam_mensaje, tam_header = sizeof(int);
	void* mensaje;
	int protocolo;

	switch(solicitud->instruccion){
	case OPERACION_SET:{
		protocolo = OPERACION_SET;
		int tam_clave = strlen(solicitud->clave) + 1;
		int tam_valor = strlen(solicitud->valor) + 1;

		tam_mensaje = tam_header + sizeof(int) + tam_clave + sizeof(int) + tam_valor;

		void* mensaje = malloc(tam_mensaje);
		char* aux = mensaje;

		memcpy(aux, &solicitud->instruccion, tam_header);
		aux += tam_header;

		memcpy(aux, &tam_clave, sizeof(int));
		aux += sizeof(int);

		memcpy(aux, solicitud->clave, tam_clave);
		aux += tam_clave;

		memcpy(aux, &tam_valor, sizeof(int));
		aux += sizeof(int);

		memcpy(aux, solicitud->valor, tam_valor);

		break;
	}
	case OPERACION_STORE:
		break;
	default:
		;
	}

	return enviar_paquete(protocolo, socket, tam_mensaje, mensaje);
}

