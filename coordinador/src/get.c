/*
 * get.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "get.h"

t_solicitud* crear_get(int socket, int id){
	t_solicitud* solicitud = crear_solicitud(OPERACION_GET, id, socket);

	solicitud->clave = recibir_string(socket);

	return solicitud;
}

t_mensaje serializar_get_a_instancia(char* clave){
	int tam_clave = strlen(clave) + 1;

	t_mensaje mensaje = crear_mensaje(CREAR_CLAVE, sizeof(int) + tam_clave);

	serializar_string(mensaje.payload, clave);

	return mensaje;
}

int realizar_get(t_solicitud* solicitud){
	bool existe_clave = existe_clave_en_instancia_activa(solicitud);

	if(validar_existencia_clave(solicitud) == -1) {
		abortar_esi(solicitud);

		log_error(LOG_COORD, "No se pudo ejecutar el get del esi %d", solicitud->id_esi);

		return -1;
	}

	log_trace(LOG_COORD, "Se valido la existencia de la clave %s", solicitud->clave);

	/**
	 * FIXME aca el problema es que, si la clave no existia, entonces
	 * en la funcion validar_existencia_clave() se crea la clave.
	 * Pero al ser una funcion que habla con la instancia, esta podria
	 * tener algun error, entonces aca no tendria que enviarle el get
	 * al planif sino manejar este error.
	 * Una solucion podria ser primero checkear la respuesta de la instance,
	 * y despues mandarle el get al planificador	 */

	if(!existe_clave){//TODO ver que onda con la falta de espacio
		sem_wait(&solicitud->solicitud_finalizada);

		if(solicitud->resultado_instancia == ERROR_DE_COMUNICACION){
			solicitud->respuesta_a_esi = ERROR_DE_COMUNICACION;

			abortar_esi(solicitud);

			log_error(LOG_COORD,
				"ocurrio un error de comunicacion con la instancia al hacer un get de la clave %s del esi %d",
				solicitud->clave,
				solicitud->id_esi);

			return -1;
		}
	}

	t_mensaje get = serializar_get_a_planif(solicitud);

	if(enviar_a_planif(get) < 0){
		log_error(LOG_COORD, "No se pudo enviar el get del esi %d de la clave %s al planificador",
				solicitud->id_esi,
				solicitud->clave
		);

		solicitud->respuesta_a_esi = ERROR_DE_COMUNICACION;

		abortar_esi(solicitud);

		log_error(LOG_COORD, "error de comunicacion con el planif");

		return -1;
	}

	log_trace(LOG_COORD, "Se envio el get %s al planificador", solicitud->clave);

	int resultado_planif = recibir_protocolo(SOCKET_PLANIF);

	if(resultado_planif <= 0){
		log_error(LOG_COORD, "No se pudo recibir el resultado del get del esi %d de la clave %s desde el planificador",
				solicitud->id_esi,
				solicitud->clave
		);

		solicitud->respuesta_a_esi = ERROR_DE_COMUNICACION;

		abortar_esi(solicitud);

		log_error(LOG_COORD, "error de comunicacion con el planif");

		return -1;
	}

	solicitud->respuesta_a_esi = resultado_planif;

	log_info(LOG_COORD, "El resultado de ejecutar la instruccion %d fue %d",
			solicitud->instruccion,
			solicitud->respuesta_a_esi);

	return 0;
}

t_mensaje serializar_get_a_planif(t_solicitud* solicitud){
	int tam_clave = strlen(solicitud->clave) + 1;

	t_mensaje mensaje = crear_mensaje(GET_CLAVE, sizeof(int) * 2 + tam_clave);

	char* aux = mensaje.payload;

	memcpy(aux, &solicitud->id_esi, sizeof(int));
	aux += sizeof(int);

	memcpy(aux, &tam_clave, sizeof(int));
	aux += sizeof(int);

	memcpy(aux, solicitud->clave, tam_clave);

	return mensaje;
}

void crear_clave(t_solicitud* solicitud){
	distribuir(solicitud);
}

int validar_existencia_clave(t_solicitud* solicitud){
	t_instancia* instancia = instancia_con_clave(solicitud);

	if(instancia != NULL){
		if(!esta_activa(instancia)){
			borrar_clave(solicitud, instancia);

			log_info(LOG_COORD,"Se borro la clave de la instancia %d");

			setear_error_clave_inaccesible(solicitud);

			return -1;
		} else
			log_info(LOG_COORD, "La clave %s se encuentra en una instancia activa", solicitud->clave);
	}
	else {
		crear_clave(solicitud);

		log_info(LOG_COORD, "La clave %s no existia y se creo en una instancia", solicitud->clave);
	}

	return 0;
}

bool existe_clave_en_instancia_activa(t_solicitud* solicitud){
	t_instancia* instancia = instancia_con_clave(solicitud);

	return instancia != NULL && esta_activa(instancia);
}

void abortar_esi(t_solicitud* solicitud){
	if(enviar_paquete(solicitud->respuesta_a_esi, (int) solicitud->socket_esi, 0, NULL) <= 0){ // FIXME ENORME
		log_error(
			LOG_COORD,
			"No se pudo enviar el resultado de la instruccion %d al esi %d",
			solicitud->instruccion,
			solicitud->id_esi
		);
	}
}

void setear_respuesta_a_esi(t_solicitud* solicitud, int resultado_planif){

	switch (solicitud->resultado_instancia){
	case ERROR_CLAVE_INACCESIBLE:
		solicitud->respuesta_a_esi = ERROR_CLAVE_INACCESIBLE;
		break;
	case ERROR_DE_COMUNICACION:
		solicitud->respuesta_a_esi = ERROR_DE_COMUNICACION;
		break;
	case ERROR_CLAVE_NO_IDENTIFICADA:
//	case FG_EI:
//	case FG_NC:
	default:
		solicitud->respuesta_a_esi = resultado_planif;
	}
}





