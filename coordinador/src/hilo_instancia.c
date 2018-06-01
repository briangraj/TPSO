/*
 * hilo_instancia.c
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#include "hilo_instancia.h"

void crear_hilo_instancia(t_instancia* instancia){
	CANTIDAD_ENTRADAS_TOTALES = 20;
	TAMANIO_ENTRADA = 100;

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
		pedido = (t_solicitud*)queue_pop(instancia->pedidos);

		enviar_pedido(pedido, instancia->socket);
	}

	return NULL;
}

void enviar_configuracion_instancia(int socket_instancia){
	int tamanio_payload = sizeof(CANTIDAD_ENTRADAS_TOTALES) + sizeof(TAMANIO_ENTRADA);
	void* payload = malloc(tamanio_payload);

	memcpy(payload, &CANTIDAD_ENTRADAS_TOTALES, sizeof(CANTIDAD_ENTRADAS_TOTALES));
	memcpy(payload + sizeof(CANTIDAD_ENTRADAS_TOTALES), &TAMANIO_ENTRADA, sizeof(TAMANIO_ENTRADA));

	enviar_paquete(CONFIGURACION_ENTRADAS, socket_instancia, tamanio_payload, payload);
}

int enviar_pedido(t_solicitud* solicitud, int socket){
	int tam_mensaje, tam_header = sizeof(int);
	void* mensaje;
	int protocolo;

	//TODO arreglar repeticion de logica
	switch(solicitud->instruccion){
	case OPERACION_SET:{
		protocolo = OPERACION_SET;
		int tam_clave = strlen(solicitud->clave) + 1;
		int tam_valor = strlen(solicitud->valor) + 1;

		tam_mensaje = tam_header + sizeof(int) + tam_clave + sizeof(int) + tam_valor;

		void* mensaje = malloc(tam_mensaje);
		void* aux = mensaje;

		memcpy(aux, &solicitud->instruccion, tam_header);
		aux += tam_header;

		serializar_string(aux, solicitud->clave);
		aux += sizeof(int) + tam_clave;

		serializar_string(aux, solicitud->valor);

		break;
	}
	case OPERACION_STORE:{
		protocolo = OPERACION_SET;
		int tam_clave = strlen(solicitud->clave) + 1;

		tam_mensaje = tam_header + sizeof(int) + tam_clave;

		void* mensaje = malloc(tam_mensaje);
		void* aux = mensaje;

		memcpy(aux, &solicitud->instruccion, tam_header);
		aux += tam_header;

		serializar_string(aux, solicitud->valor);

		break;
	}
	default:
		return -1;
	}

	return enviar_paquete(protocolo, socket, tam_mensaje, mensaje);
}

