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
	int id_esi = recibir_id(socket_esi);

	if(id_esi == -1){
		log_error(LOG_COORD, "ERROR: no se pudo recibir el id del esi en el socket %d", socket_esi);
		return -1;
	}

	while(true){
		t_solicitud* solicitud = recibir_solicitud_esi((int) socket_esi, id_esi);

		realizar_solicitud(solicitud); //TODO chequear si necesita error

		if(enviar_paquete(solicitud->respuesta_a_esi, socket_esi, 0, NULL) <= 0){
			log_error(LOG_COORD, "no se pudo enviar el resultado del get de la clave %s al esi %d",
					solicitud->clave,
					solicitud->id_esi
			);
			return 1 * -1;
		}
	}

	return 0;
}

void realizar_solicitud(t_solicitud* solicitud){
	switch(solicitud->instruccion){
	case OPERACION_GET:
		realizar_get(solicitud);
		break;
	case OPERACION_SET:
		realizar_set(solicitud);
		break;
	case OPERACION_STORE:
		realizar_store(solicitud);
		break;
	}
}


int enviar_a_planif(int protocolo, void* payload, int tam_payload){
	pthread_mutex_lock(&SEM_SOCKET_PLANIF);

	int resultado_envio = enviar_paquete(protocolo, SOCKET_PLANIF, tam_payload, payload);

	pthread_mutex_unlock(&SEM_SOCKET_PLANIF);

	return resultado_envio;
}

void agregar_pedido(t_instancia* instancia, t_solicitud* solicitud){
	//faltarian semaforos
	queue_push(instancia->pedidos, solicitud);
	sem_post(&instancia->sem);
}

t_solicitud* recibir_solicitud_esi(int socket, int id){

	int protocolo = recibir_protocolo(socket);

	t_solicitud* solicitud;

	if(protocolo == -1){
		log_error(LOG_COORD, "El esi %d envio una solicitud invalida", socket);
		pthread_exit(NULL);
	}

	sem_init(&solicitud->solicitud_finalizada, 0, 0);

	switch(protocolo){

	case OPERACION_GET:
		solicitud = crear_get(socket);
		break;
	case OPERACION_SET:
		solicitud = crear_set(socket);
		break;
	case OPERACION_STORE:
		solicitud = crear_store(socket);
		break;
	default:
		log_error(LOG_COORD, "El protocolo %d no existe", protocolo);
		pthread_exit(NULL);
		return NULL;
	}

	solicitud->id_esi = id;

	return solicitud;

}

int recibir_id(int socket){
	int protocolo = recibir_protocolo(socket);

	if(protocolo != ENVIO_ID)
		return -1;

	int id;

	int bytes_recibidos = recv(socket, &id, sizeof(int), MSG_WAITALL);

	if(bytes_recibidos <= 0)
		return -1;

	return id;
}
