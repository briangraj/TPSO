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

void setear_error_comunicacion_instancia(t_solicitud* solicitud){
	solicitud->resultado_instancia = ERROR_DE_COMUNICACION;
}

void setear_error_clave_inaccesible(t_solicitud* solicitud){
	solicitud->respuesta_a_esi = ERROR_CLAVE_INACCESIBLE;
}

void setear_operacion_exitosa_instancia(t_solicitud* solicitud){
	solicitud->resultado_instancia = OPERACION_EXITOSA;
}

void destruir_solicitud(t_solicitud* solicitud){
	free(solicitud->clave);
	free(solicitud->valor);
	sem_close(&solicitud->solicitud_finalizada);
	sem_destroy(&solicitud->solicitud_finalizada);
	free(solicitud);
}


void log_error_envio_planif(t_solicitud* solicitud){
	switch(solicitud->instruccion){

	case OPERACION_GET:
		log_error(LOG_COORD, "No se pudo enviar el get del esi %d de la clave %s al planificador",
						solicitud->id_esi,
						solicitud->clave);
		break;

	case OPERACION_SET:
		log_error(LOG_COORD, "No se pudo enviar el mensaje %s %s del esi %d al planificador",
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

	case OPERACION_SET:
		log_error(LOG_COORD, "No se pudo recibir el resultado del set %s %s del esi %d desde el planificador",
					solicitud->clave,
					solicitud->valor,
					solicitud->id_esi
			);
		break;
	}

}









