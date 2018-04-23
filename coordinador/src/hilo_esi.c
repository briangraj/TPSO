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

void enviar_solicitud(t_solicitud* solicitud, t_instancia* instancia){//TODO mock
	int tam_mensaje, tam_header = sizeof(int);
	void* mensaje;

	switch(solicitud->instruccion){
	case SET:{
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
	case STORE:
		break;
	default:
		;
	}

	send_all(instancia->socket, (void*) mensaje, tam_mensaje);
}

void distribuir(t_solicitud* solicitud){
	t_instancia* instancia = elegir_instancia(solicitud);//TODO mock
	enviar_solicitud(solicitud, instancia);//TODO mock
}

t_solicitud* recibir_solicitud_esi(int socket){

	int protocolo = recibir_protocolo(socket);

	if(protocolo == -1){
		log_error(log_coord, "El esi %d envio una solicitud invalida", socket);
		pthread_exit(NULL);
	}

	switch(protocolo){

	case GET:
		return crear_get(socket);
	case SET:
		return crear_set(socket);
	case STORE:
		return crear_store(socket);
	default:
		log_error(log_coord, "El protocolo %d no existe", protocolo);
		pthread_exit(NULL);
		return NULL;
	}

}

t_solicitud* crear_get(int socket){
	t_solicitud* solicitud = malloc(sizeof(t_solicitud));

	solicitud->instruccion = GET;
	solicitud->clave = recibir_string(socket);

	return solicitud;
}

t_solicitud* crear_set(int socket){
	t_solicitud* solicitud = malloc(sizeof(t_solicitud));

	solicitud->instruccion = SET;
	solicitud->clave = recibir_string(socket);
	solicitud->valor = recibir_string(socket);

	return solicitud;
}

t_solicitud* crear_store(int socket){
	t_solicitud* solicitud = malloc(sizeof(t_solicitud));

	solicitud->instruccion = GET;
	solicitud->clave = recibir_string(socket);

	return solicitud;
}

char* recibir_string(int socket){
	int tam_clave;

	recv(socket, &tam_clave, sizeof(int), MSG_WAITALL);

	char* clave = malloc(tam_clave);

	recv(socket, clave, tam_clave, MSG_WAITALL);

	return clave;
}





























