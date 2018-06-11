/*
 * set.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "set.h"

t_solicitud* crear_set(int socket, int id){
	t_solicitud* solicitud = crear_solicitud(OPERACION_SET, id, socket);

	solicitud->clave = recibir_string(socket);
	solicitud->valor = recibir_string(socket);

	return solicitud;
}

t_mensaje serializar_set_a_instancia(char* clave, char* valor){
	int tam_clave = strlen(clave) + 1;
	int tam_valor = strlen(valor) + 1;

	int tam_payload = sizeof(int) + tam_clave + sizeof(int) + tam_valor;

	t_mensaje mensaje = crear_mensaje(OPERACION_SET, tam_payload);

	serializar_string(mensaje.payload, clave);

	serializar_string(mensaje.payload + sizeof(int) + tam_clave, valor);

	return mensaje;
}

t_mensaje serializar_set_a_planif(t_solicitud* solicitud){
	int tam_payload = sizeof(int) * 3 + strlen(solicitud->clave) + strlen(solicitud->valor) + 2;

	t_mensaje mensaje = crear_mensaje(GET_CLAVE, tam_payload);

	char* aux = mensaje.payload;

	memcpy(aux, &solicitud->id_esi, sizeof(int));
	aux += sizeof(int);

	serializar_string(aux, solicitud->clave);
	aux += strlen(solicitud->clave) + 1;

	serializar_string(aux, solicitud->valor);

	return mensaje;
}

int realizar_set(t_solicitud* solicitud){
	t_instancia* instancia = instancia_con_clave(solicitud);

	if(instancia == NULL){
		solicitud->respuesta_a_esi = ERROR_CLAVE_NO_IDENTIFICADA;

		abortar_esi(solicitud);

		log_error(LOG_COORD, "No se encontro la clave %s, se abortara al esi %d", solicitud->clave, solicitud->id_esi);

		return -1;
	} else if(!esta_activa(instancia)){
		solicitud->respuesta_a_esi = ERROR_CLAVE_INACCESIBLE;
		//TODO hay que borrar la clave
		abortar_esi(solicitud);

		log_error(LOG_COORD, "La clave %s se encuentra en una instancia desconectada, se abortara al esi %d",
				solicitud->clave,
				solicitud->id_esi);

		return -1;
	}

	agregar_solicitud(instancia, solicitud);

	sem_wait(&solicitud->solicitud_finalizada);

	if(solicitud->resultado_instancia == ERROR_DE_COMUNICACION){
		/**
		 * TODO falta checkear FS_EI, FS_NC, y ver que evento
		 * desencadena la compactacion
		 */
		solicitud->respuesta_a_esi = ERROR_DE_COMUNICACION;

		abortar_esi(solicitud);

		log_error(LOG_COORD,
			"ocurrio un error de comunicacion con la instancia al hacer un set %s %s del esi %d",
			solicitud->clave,
			solicitud->valor,
			solicitud->id_esi);

		return -1;
	}

	t_mensaje set = serializar_set_a_planif(solicitud);

	if(enviar_a_planif(set) < 0){
		log_error(LOG_COORD, "No se pudo enviar el set %s %s del esi %d al planificador",
				solicitud->clave,
				solicitud->valor,
				solicitud->id_esi
		);

		solicitud->respuesta_a_esi = ERROR_DE_COMUNICACION;

		abortar_esi(solicitud);

		return -1;
	}

	log_trace(LOG_COORD, "Se envio el set %s %s al planificador", solicitud->clave, solicitud->valor);

	int resultado_planif = recibir_protocolo(SOCKET_PLANIF);

	if(resultado_planif <= 0){
		log_error(LOG_COORD, "No se pudo recibir el resultado del set %s %s del esi %d desde el planificador",
				solicitud->clave,
				solicitud->valor,
				solicitud->id_esi
		);

		solicitud->respuesta_a_esi = ERROR_DE_COMUNICACION;

		abortar_esi(solicitud);

		return -1;
	}

	solicitud->respuesta_a_esi = resultado_planif;

	log_info(LOG_COORD, "El resultado de ejecutar la instruccion %d fue %d",
			solicitud->instruccion,
			solicitud->respuesta_a_esi);

	return 0;
}
