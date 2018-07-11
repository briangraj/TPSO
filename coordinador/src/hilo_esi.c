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
		log_error(LOG_COORD, "No se pudo recibir el id del esi en el socket %d", socket_esi);
		pthread_exit(NULL);
	}

	log_info(LOG_COORD, "Se recibio al esi de id %d", id_esi);

	while(true){
		t_solicitud* solicitud = recibir_solicitud_esi((int) socket_esi, id_esi);

		if(solicitud == NULL){
			log_error(LOG_COORD, "No se pudo crear la solicitud del esi %d", id_esi);
			break;
		}

		if(solicitud->instruccion == FIN_DEL_SCRIPT){
			log_trace(LOG_COORD, "El esi %d termino el script", id_esi);
			destruir_solicitud(solicitud);
			break;
		}

		if(atender_solicitud(solicitud) == -1){
			log_error(LOG_COORD, "No se pudo atender la solicitud del esi %d, se abortara", solicitud->id_esi);

			abortar_esi(solicitud);

			destruir_solicitud(solicitud);

			verificar_estado_valido();
			break;
		}

		log_trace(LOG_COORD, "Se atendio la instruccion %d del esi %d", solicitud->instruccion, solicitud->id_esi);

		if(enviar_paquete(solicitud->respuesta_a_esi, (int) socket_esi, 0, NULL) <= 0){
			log_error(
					LOG_COORD,
					"No se pudo enviar el resultado de la instruccion %d al esi %d",
					solicitud->instruccion,
					solicitud->id_esi
			);

			destruir_solicitud(solicitud);
			break;
		}

		log_info(LOG_COORD, "Se envio la respuesta %d al esi %d", solicitud->respuesta_a_esi, solicitud->id_esi);

		liberar_solicitud(solicitud);
	}

	pthread_exit(NULL);
}

int atender_solicitud(t_solicitud* solicitud){
	sleep(RETARDO);

	switch(solicitud->instruccion){
	case OPERACION_GET:
		log_trace(LOG_COORD, "Se ejecutara un GET %s", solicitud->clave);

		return get(solicitud);
	case OPERACION_SET:
		log_trace(LOG_COORD, "Se ejecutara un SET %s %s", solicitud->clave, solicitud->valor);

		return set(solicitud);
	case OPERACION_STORE:
		log_trace(LOG_COORD, "Se ejecutara un STORE %s", solicitud->clave);

		return store(solicitud);
	default:
		return -1;
	}
}

t_solicitud* recibir_solicitud_esi(int socket, int id){
	t_solicitud* solicitud;

	int protocolo = recibir_protocolo(socket);

	if(protocolo == FIN_DEL_SCRIPT){
		solicitud = malloc(sizeof(t_solicitud));
		solicitud->instruccion = FIN_DEL_SCRIPT;
		return solicitud;
	}

	if(protocolo == -1){
		log_info(LOG_COORD, "Se desconecto el esi %d", id);
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

void verificar_estado_valido(){
	if(!PLANIF_CONECTADO){
		log_error(LOG_COORD, "Se desconecto el planificador, se abortara el coordinador");

		close(LISTENER);
		destruir_instancias();
		list_destroy(INSTANCIAS);
		free(ALGORITMO_DISTRIBUCION);
		free(IP_COORD);
		log_destroy(LOG_COORD);

		exit(EXIT_FAILURE);
	}
}

int ejecutar(t_solicitud* solicitud, t_instancia* instancia){
	agregar_solicitud(instancia, solicitud);

	sem_wait(&solicitud->solicitud_finalizada);

	if(validar_resultado_instancia(solicitud, instancia) == -1)
		return -1;

	return 0;
}

int enviar_a_planif(t_solicitud* solicitud){
	t_mensaje mensaje = serializar_a_planif(solicitud);

	if(resultado_enviar_a_planif(mensaje, solicitud) == -1)
		return -1;

	log_trace(LOG_COORD, "Se envio al planificador");

	if(validar_resultado_planif(solicitud) == -1)
		return -1;

	return 0;
}

t_mensaje serializar_a_planif(t_solicitud* solicitud){
	switch(solicitud->instruccion){
	case OPERACION_SET:
		return serializar_set_a_planif(solicitud);
	default:
		return serializar_clave_a_planif(solicitud);
	}
}

int checkear_clave_valida(t_instancia* instancia, t_solicitud* solicitud){
	if(instancia == NULL){
		set_respuesta_a_esi(solicitud, ERROR_CLAVE_NO_IDENTIFICADA);

		log_error(LOG_COORD, "ERROR_CLAVE_NO_IDENTIFICADA: No se encontro la clave %s, se abortara al esi %d", solicitud->clave, solicitud->id_esi);

		return -1;
	} else if(!esta_activa(instancia)){
		set_respuesta_a_esi(solicitud, ERROR_CLAVE_INACCESIBLE);

		agregar_clave_a_borrar(instancia, solicitud->clave);

		log_error(LOG_COORD, "ERROR_CLAVE_INACCESIBLE: La clave %s se encuentra en una instancia desconectada, se abortara al esi %d",
				solicitud->clave,
				solicitud->id_esi);

		return -1;
	}

	return 0;
}
