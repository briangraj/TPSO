/*
 * hilo_instancia.c
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#include "hilo_instancia.h"

void crear_hilo_instancia(t_instancia* instancia){
	crear_hilo(atender_instancia, (void*) instancia);
}

void* atender_instancia(void* instancia_void){
	t_instancia* instancia = (t_instancia*) instancia_void;
	t_solicitud* solicitud;

	instancia->id_hilo = pthread_self();//FIXME al final usamos esto? o se puede ir?

	while(true) {
		sem_wait(&instancia->solicitud_lista);

		solicitud = sacar_solicitud(instancia);

		if(enviar_solicitud(solicitud, instancia->socket_instancia) == -1){
			log_error(
					LOG_COORD,
					"No se pudo enviar la solicitud de la instruccion %d a la instancia %d",
					solicitud->instruccion,
					instancia->id
			);

			if(solicitud->instruccion != COMPACTACION){
				setear_error_instancia_inactiva(solicitud);

				agregar_clave_a_borrar(instancia, solicitud->clave);
			} else
				solicitud->resultado_instancia = ERROR_DE_COMUNICACION;

			desconectar_instancia(instancia);

			pthread_exit(NULL);
		}

		log_info(LOG_COORD, "Se envio la solicitud de la instruccion %d a la instancia %d",
			solicitud->instruccion,
			instancia->id);

		evaluar_resultado_instr(solicitud, instancia);

		sem_post(&solicitud->solicitud_finalizada);

		log_trace(LOG_COORD, "El resultado de la instruccion %d fue %d", solicitud->instruccion, solicitud->respuesta_a_esi);
	}

	pthread_exit(NULL);
}

void evaluar_resultado_instr(t_solicitud* solicitud, t_instancia* instancia){
	switch(recibir_protocolo(instancia->socket_instancia)){
	case OPERACION_EXITOSA:
		setear_operacion_exitosa_instancia(solicitud);
		break;
	case FS_NC:
		solicitud->respuesta_a_esi = FS_NC;
		break;
	case FS_EI:
		solicitud->respuesta_a_esi = FS_EI;
		break;
	default:
		if(solicitud->instruccion != COMPACTACION){
			setear_error_instancia_inactiva(solicitud);
			agregar_clave_a_borrar(instancia, solicitud->clave);
		} else
			solicitud->resultado_instancia = ERROR_DE_COMUNICACION;

		desconectar_instancia(instancia);
	}
}

t_mensaje serializar_config_instancia(){
	t_mensaje config = crear_mensaje(CONFIGURACION_ENTRADAS, 2 * sizeof(int));

	memcpy(config.payload, &CANTIDAD_ENTRADAS_TOTALES, sizeof(int));
	memcpy(config.payload + sizeof(int), &TAMANIO_ENTRADA, sizeof(int));

	return config;
}

int enviar_config_instancia(t_instancia* instancia){
	t_mensaje config = serializar_config_instancia();

	return enviar_mensaje(config, instancia->socket_instancia);
}

int enviar_solicitud(t_solicitud* solicitud, int socket){
	t_mensaje mensaje;

	switch(solicitud->instruccion){
	
	case CREAR_CLAVE:{
		mensaje = serializar_set_a_instancia(solicitud);

		break;
	}
	
	case OPERACION_SET:{
		mensaje = serializar_set_a_instancia(solicitud);

		break;
	}
	case OPERACION_STORE:{
		mensaje = serializar_store_a_instancia(solicitud);

		break;
	}

	case COMPACTACION:{
		mensaje = serializar_compactacion(solicitud);
		break;
	}

	default:
		return -1;
	}

	return enviar_mensaje(mensaje, socket);
}




