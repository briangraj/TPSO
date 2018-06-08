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
	int id_esi = recibir_id((int) socket_esi);

	if(id_esi == -1){
		log_error(LOG_COORD, "no se pudo recibir el id del esi en el socket %d", socket_esi);
		pthread_exit(NULL);
	}

	log_info(LOG_COORD, "Se recibio al esi de id %d", id_esi);

	while(true){
		t_solicitud* solicitud = recibir_solicitud_esi((int) socket_esi, id_esi);

		if(solicitud == NULL){
			log_error(LOG_COORD, "No se pudo crear la solicitud del esi %d", id_esi);
			pthread_exit(NULL);
		}

		if(atender_solicitud(solicitud) == -1){
			log_error(LOG_COORD, "no se pudo atender la solicitud del esi %d", solicitud->id_esi);
			pthread_exit(NULL);
		}

		log_trace(LOG_COORD, "Se recibio la solicitud %d del esi %d", solicitud->instruccion, solicitud->id_esi);

		if(enviar_paquete(solicitud->respuesta_a_esi, (int) socket_esi, 0, NULL) <= 0){
			log_error(
					LOG_COORD,
					"no se pudo enviar el resultado de la instruccion %d al esi %d",
					solicitud->instruccion,
					solicitud->id_esi
			);

			pthread_exit(NULL);
		}

		log_info(LOG_COORD, "Se envio la respuesta %d al esi %d", solicitud->respuesta_a_esi, solicitud->id_esi);
	}

	pthread_exit(NULL);
}

int atender_solicitud(t_solicitud* solicitud){
	switch(solicitud->instruccion){
	case OPERACION_GET:
		return realizar_get(solicitud);
	case OPERACION_SET:
		return realizar_set(solicitud);
	case OPERACION_STORE:
		return realizar_store(solicitud);
	default:
		return -1;
	}
}

int enviar_a_planif(t_mensaje mensaje){
	pthread_mutex_lock(&SEM_SOCKET_PLANIF);

	int resultado_envio = enviar_mensaje(mensaje, SOCKET_PLANIF);

	pthread_mutex_unlock(&SEM_SOCKET_PLANIF);

	return resultado_envio;
}

t_solicitud* recibir_solicitud_esi(int socket, int id){

	int protocolo = recibir_protocolo(socket);

	t_solicitud* solicitud;

	if(protocolo == -1){
		log_error(LOG_COORD, "el esi %d envio una solicitud invalida", socket);
		return NULL;
	}

	switch(protocolo){

	case OPERACION_GET:
		solicitud = crear_get(socket, id);
		break;
	case OPERACION_SET:
		solicitud = crear_set(socket, id);
		break;
	case OPERACION_STORE:
		solicitud = crear_store(socket, id);
		break;
	default:
		log_error(LOG_COORD, "El protocolo %d no existe", protocolo);
		return NULL;
	}

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
