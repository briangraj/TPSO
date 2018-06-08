/*
 * get.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "get.h"

t_solicitud* crear_get(int socket, int id){
	t_solicitud* solicitud = crear_solicitud(OPERACION_GET, id);

	solicitud->clave = recibir_string(socket);

	return solicitud;
}
t_mensaje serializar_get_a_instancia(char* clave){
	int tam_clave = strlen(clave) + 1;

	t_mensaje mensaje = crear_mensaje(CREAR_CLAVE, sizeof(int) + tam_clave);

	serializar_string(mensaje.payload, clave);

	return mensaje;
}

int evaluar_resultados(int resultado_instancia, int resultado_planif){
	return 0;//TODO mock
}

int realizar_get(t_solicitud* solicitud){
	validar_existencia_clave(solicitud);
	log_trace(LOG_COORD, "Se valido la existencia de la clave %s", solicitud->clave);

	/**
	 * FIXME aca el problema es que, si la clave no existia, entonces
	 * en la funcion validar_existencia_clave() se crea la clave.
	 * Pero al ser una funcion que habla con la instancia, esta podria
	 * tener algun error, entonces aca no tendria que enviarle el get
	 * al planif sino manejar este error.
	 * Una solucion podria ser primero checkear la respuesta de la instancia,
	 * y despues mandarle el get al planificador
	 */

	t_mensaje get = serializar_get_a_planif(solicitud);

	if(enviar_a_planif(get) < 0){
		log_error(LOG_COORD, "No se pudo enviar el get del esi %d de la clave %s al planificador",
				solicitud->id_esi,
				solicitud->clave
		);
		return -1;
	}

	log_trace(LOG_COORD, "Se envio el get %s al planificador", solicitud->clave);

	int resultado_planif = recibir_protocolo(SOCKET_PLANIF);

	if(resultado_planif <= 0){
		log_error(LOG_COORD, "No se pudo recibir el resultado del get del esi %d de la clave %s desde el planificador",
				solicitud->id_esi,
				solicitud->clave
		);
		return -1;
	}

	sem_wait(&solicitud->solicitud_finalizada);
	solicitud->respuesta_a_esi = evaluar_resultados(solicitud->resultado_instancia, resultado_planif);

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

void agregar_clave_a_borrar(t_solicitud* solicitud, t_list* instancias){
	bool no_contiene_clave_a_borrar(char* clave){
		return strcmp(clave, solicitud->clave);
	}

	bool no_contiene_clave_en_claves_a_borrar(t_instancia* instancia){
		return list_any_satisfy(instancia->claves_a_borrar, no_contiene_clave_a_borrar);
	}

	t_instancia* instancia = list_find(instancias, no_contiene_clave_en_claves_a_borrar);
}

void crear_clave_con_instancia_caida(t_solicitud* solicitud, t_list* instancias){
	distribuir(solicitud);

	agregar_clave_a_borrar(solicitud, instancias);
}

void crear_clave(t_solicitud* solicitud){
	distribuir(solicitud);
}

void validar_existencia_clave(t_solicitud* solicitud){
	t_list* instancias = instancias_con_clave(solicitud);

	if(!list_is_empty(instancias)){
		if(!list_any_satisfy(instancias, es_instancia_activa)){
			crear_clave_con_instancia_caida(solicitud, instancias);
			log_info(LOG_COORD,"Se creo la clave con instancia caida");
		} else
			log_info(LOG_COORD, "La clave %s se encuentra en una instancia activa", solicitud->clave);
	}
	else {
		crear_clave(solicitud);
		log_info(LOG_COORD, "La clave %s no existia y se creo en una instancia", solicitud->clave);
	}

}

t_list* instancias_con_clave(t_solicitud* solicitud){

	bool instancia_contiene_clave(t_instancia* instancia){
		bool contiene_clave(char* clave){
			return !strcmp(clave, (t_instancia*) solicitud->clave);
		}

		return list_any_satisfy(instancia->claves, contiene_clave);
	}

	return list_filter(INSTANCIAS, instancia_contiene_clave);
}
