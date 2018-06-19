/*
 * get.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "get.h"

int realizar_get(t_solicitud* solicitud){
	if(validar_existencia_clave(solicitud) == -1) { //TODO capaz no hace falta chequear
		log_error(LOG_COORD, "No se pudo ejecutar el get del esi %d", solicitud->id_esi);

		return -1;
	}

	log_trace(LOG_COORD, "Se valido la existencia de la clave %s", solicitud->clave);

	t_mensaje get = serializar_get_a_planif(solicitud);

	if(resultado_enviar_a_planif(get, solicitud) == -1){
		return -1;
	}

	log_trace(LOG_COORD, "Se envio el get %s al planificador", solicitud->clave);

	if(validar_resultado_planif(solicitud) == -1)
		return -1;

	log_info(LOG_COORD, "El resultado de ejecutar la instruccion %d fue %d",
			solicitud->instruccion,
			solicitud->respuesta_a_esi);

	return 0;
}


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

int validar_existencia_clave(t_solicitud* solicitud){
	t_instancia* instancia = instancia_con_clave(solicitud);

	if(instancia == NULL){
		agregar_clave_a_crear(distribuir(solicitud), solicitud->clave);
		/**
		 * TODO
		 * if(hay_error_en_distribuir())
		 * 		solicitud->respuesta_a_esi = ERROR_DE_COMUNICACION;
		 * 		return -algo;
		 */

		log_info(LOG_COORD, "La clave %s no existia y se creo en una instancia", solicitud->clave);
	} else
		log_info(LOG_COORD, "La clave %s se encuentra en la instancia %d", solicitud->clave, instancia->id);

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





