/*
 * hilo_esi.c
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#include "hilo_esi.h"

void crear_hilo_esi(int socket_cliente){

	crear_hilo(atender_esi, (void*) socket_cliente);

}

void* atender_esi(void* socket_esi){
	while(true){
		t_solicitud* solicitud = recibir_solicitud_esi((int) socket_esi);
		distribuir(solicitud); //todo chequear si necesita error
	}

	return 0;
}

t_instancia* elegir_instancia(t_solicitud* solicitud){//TODO mock
	t_instancia* instancia = NULL;

	return instancia;
}

void distribuir(t_solicitud* solicitud){
	t_instancia* instancia = elegir_instancia(solicitud);//TODO mock

	agregar_pedido(instancia, solicitud);
	sem_post(&instancia->sem);
	//enviar_solicitud(solicitud, instancia);//TODO mock
}

void agregar_pedido(t_instancia* instancia, t_solicitud* solicitud){
	//faltarian semaforos
	list_add(instancia->pedidos, solicitud);
}

t_solicitud* recibir_solicitud_esi(int socket){

	int protocolo = recibir_protocolo(socket);

	if(protocolo == -1){
		log_error(log_coord, "El esi %d envio una solicitud invalida", socket);
		pthread_exit(NULL);
	}

	switch(protocolo){

	case OPERACION_GET:
		return crear_get(socket);
	case OPERACION_SET:
		return crear_set(socket);
	case OPERACION_STORE:
		return crear_store(socket);
	default:
		log_error(log_coord, "El protocolo %d no existe", protocolo);
		pthread_exit(NULL);
		return NULL;
	}

}

t_solicitud* crear_get(int socket){
	t_solicitud* solicitud = malloc(sizeof(t_solicitud));

	solicitud->instruccion = OPERACION_GET;
	solicitud->clave = recibir_string(socket);
	solicitud->socket_esi = socket;

	return solicitud;
}

t_solicitud* crear_set(int socket){
	t_solicitud* solicitud = malloc(sizeof(t_solicitud));

	solicitud->instruccion = OPERACION_SET;
	solicitud->clave = recibir_string(socket);
	solicitud->valor = recibir_string(socket);
	solicitud->socket_esi = socket;

	return solicitud;
}

t_solicitud* crear_store(int socket){
	t_solicitud* solicitud = malloc(sizeof(t_solicitud));

	solicitud->instruccion = OPERACION_GET;
	solicitud->clave = recibir_string(socket);
	solicitud->socket_esi = socket;

	return solicitud;
}































