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

	instancia->id_hilo = pthread_self();

	while(true) {
		sem_wait(&instancia->solicitud_lista);

		solicitud = sacar_solicitud(instancia);

		if(enviar_solicitud(solicitud, instancia->socket) == -1){
			log_error(
					LOG_COORD,
					"No se pudo enviar la solicitud de la instruccion %d a la instancia %d",
					solicitud->instruccion,
					instancia->id
			);

			setear_error_comunicacion_instancia(solicitud);

			pthread_exit(NULL);
		}

		log_info(LOG_COORD, "Se envio la solicitud de la instruccion %d a la instancia %d",
			solicitud->instruccion,
			instancia->id);

		evaluar_resultado_instr(solicitud, instancia->socket);

		sem_post(&solicitud->solicitud_finalizada);

		log_trace(LOG_COORD, "El resultado de la instruccion %d fue %d", solicitud->instruccion, solicitud->respuesta_a_esi);
	}

	pthread_exit(NULL);
}

void evaluar_resultado_instr(t_solicitud* solicitud, int socket_instancia){

	switch(recibir_protocolo(socket_instancia)){
	case OPERACION_EXITOSA:
		setear_operacion_exitosa_instancia(solicitud);
	break;
	default:
		setear_error_comunicacion_instancia(solicitud);//TODO ver que errores puede tirar la instancia
	;
	}

}

t_mensaje serializar_config_instancia(){
	t_mensaje config = crear_mensaje(CONFIGURACION_ENTRADAS, 2 * sizeof(int));

	memcpy(config.payload, &CANTIDAD_ENTRADAS_TOTALES, sizeof(int));
	memcpy(config.payload + sizeof(int), &TAMANIO_ENTRADA, sizeof(int));

	return config;
}

int enviar_config_instancia(int socket_instancia){
	t_mensaje config = serializar_config_instancia();

	return enviar_mensaje(config, socket_instancia);
}

int enviar_solicitud(t_solicitud* solicitud, int socket){
	t_mensaje mensaje;

	switch(solicitud->instruccion){
	case OPERACION_GET:{
		mensaje = serializar_get_a_instancia(solicitud->clave);

		break;
	}
	case OPERACION_SET:{
		mensaje = serializar_set_a_instancia(solicitud->clave, solicitud->valor);

		break;
	}
	case OPERACION_STORE:{
		mensaje = serializar_store_a_instancia(solicitud->clave);

		break;
	}
	default:
		return -1;
	}

	return enviar_mensaje(mensaje, socket);
}




