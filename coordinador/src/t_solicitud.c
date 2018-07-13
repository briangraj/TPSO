/*
 * t_solicitud.c
 *
 *  Created on: 6 jun. 2018
 *      Author: utnso
 */
#include "t_solicitud.h"

t_solicitud* crear_solicitud(int instruccion, int id, int socket){
	t_solicitud* solicitud = malloc(sizeof(t_solicitud));

	solicitud->instruccion = instruccion;
	solicitud->id_esi = id;
	solicitud->socket_esi = socket;
	sem_init(&solicitud->solicitud_finalizada, 0, 0);
	solicitud->respuesta_a_esi = 0;
	solicitud->resultado_instancia = 0;

	return solicitud;
}

void set_resultado_instancia(t_solicitud* solicitud, int resultado){
	solicitud->resultado_instancia = resultado;
}

void set_respuesta_a_esi(t_solicitud* solicitud, int respuesta){
	solicitud->respuesta_a_esi = respuesta;
}

void destruir_solicitud(t_solicitud* solicitud){
	desconectar_cliente(solicitud->socket_esi);
	liberar_solicitud(solicitud);
}

//bool es_clave_a_crear_en_alguna_instancia(t_solicitud* solicitud){
//	bool es_clave_a_crear_aux(t_instancia* instancia){
//		return es_clave_a_crear(instancia, solicitud);
//	}
//
//	return list_any_satisfy(INSTANCIAS, (bool (*)(void*)) es_clave_a_crear_aux);
//}
//
//bool es_get_de_clave_existente(t_solicitud* solicitud){
//	return solicitud->instruccion == OPERACION_GET
//			&& es_clave_a_crear_en_alguna_instancia(solicitud);
//}
//
//bool clave_ya_esta_en_memoria(t_solicitud* solicitud){
//	return es_get_de_clave_existente(solicitud)
//		|| solicitud->instruccion == OPERACION_STORE
//		|| solicitud->instruccion == STATUS;
//}

bool valor_ya_esta_en_memoria(t_solicitud* solicitud){
	return solicitud->instruccion == OPERACION_SET
		|| solicitud->instruccion == CREAR_CLAVE
		|| solicitud->instruccion == STATUS;
}

void liberar_solicitud(t_solicitud* solicitud){
	if(valor_ya_esta_en_memoria(solicitud))
		free(solicitud->valor);

//	if(clave_ya_esta_en_memoria(solicitud))
//		free(solicitud->clave);

	/**
	 * GET
	 * 	si es de clave nueva, entonces no hay que borrarla
	 * 	sino si
	 * SET
	 * 	si es OPERACION_SET entonces la clave ya existe, entonces hay que borrarla
	 * 	si es CREAR_CLAVE no
	 * CREAR_CLAVE
	 * 	la clave que llega debe existir
	 * STORE
	 * 	la clave que llega debe existir
	 * STATUS
	 *	la clave que llega debe existir
	 * COMPACTACION
	 * 	no involucra clave
	 */

	sem_close(&solicitud->solicitud_finalizada);
	sem_destroy(&solicitud->solicitud_finalizada);
	free(solicitud);
}

void log_error_comunicacion_instancia(t_solicitud* solicitud){
	switch(solicitud->instruccion){

	case OPERACION_STORE:
		log_error(LOG_COORD,
			"ocurrio un error de comunicacion con la instancia al hacer un store %s del esi %d",
			solicitud->clave,
			solicitud->id_esi);
		break;

	case OPERACION_SET:

		log_error(LOG_COORD,
			"ocurrio un error de comunicacion con la instancia al hacer un set %s %s del esi %d",
			solicitud->clave,
			solicitud->valor,
			solicitud->id_esi);
		break;
	}



}
void log_error_envio_planif(t_solicitud* solicitud){
	switch(solicitud->instruccion){

	case OPERACION_GET:
		log_error(LOG_COORD, "No se pudo enviar el get del esi %d de la clave %s al planificador",
				solicitud->id_esi,
				solicitud->clave);
		break;

	case OPERACION_STORE:
		log_error(LOG_COORD, "No se pudo enviar el store %s del esi %d al planificador",
				solicitud->clave,
				solicitud->id_esi
		);
		break;

	case OPERACION_SET:
		log_error(LOG_COORD, "No se pudo enviar el set %s %s del esi %d al planificador",
				solicitud->clave,
				solicitud->valor,
				solicitud->id_esi
		);
		break;
	}

}

void log_error_resultado_planif(t_solicitud* solicitud){
	switch(solicitud->instruccion){

	case OPERACION_GET:
		log_error(LOG_COORD, "No se pudo recibir el resultado del get del esi %d de la clave %s desde el planificador",
				solicitud->id_esi,
				solicitud->clave
		);
		break;

	case OPERACION_STORE:
		log_error(LOG_COORD, "No se pudo recibir el resultado del store %s del esi %d desde el planificador",
				solicitud->clave,
				solicitud->id_esi
			);
		break;

	case OPERACION_SET:
		log_error(LOG_COORD, "No se pudo recibir el resultado del set %s %s del esi %d desde el planificador",
				solicitud->clave,
				solicitud->valor,
				solicitud->id_esi
			);
		break;
	}

}


int protocolo_planif(t_solicitud* solicitud){
	switch(solicitud->instruccion){

	case OPERACION_GET:
		return GET_CLAVE;

	case OPERACION_STORE:
		return STORE_CLAVE;

	case OPERACION_SET:
		return SET_CLAVE;

	case CREAR_CLAVE:
		return SET_CLAVE;

	default:
		return -1;
	}
}









