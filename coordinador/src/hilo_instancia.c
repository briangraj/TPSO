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
				set_resultado_instancia(solicitud, ERROR_CLAVE_INACCESIBLE);

				agregar_clave_a_borrar(instancia, solicitud->clave);
			} else
				set_resultado_instancia(solicitud, ERROR_DE_COMUNICACION);

			desconectar_instancia(instancia);

			pthread_exit(NULL);
		}

		log_info(LOG_COORD, "Se envio la solicitud de la instruccion %d a la instancia %d",
			solicitud->instruccion,
			instancia->id);

		evaluar_resultado_instr(solicitud, instancia);

		sem_post(&solicitud->solicitud_finalizada);

		log_trace(LOG_COORD, "El resultado de la instruccion %d fue %d", solicitud->instruccion, solicitud->resultado_instancia);
	}

	pthread_exit(NULL);
}

void evaluar_resultado_instr(t_solicitud* solicitud, t_instancia* instancia){
	int protocolo_recibido = recibir_protocolo(instancia->socket_instancia);

	log_warning(LOG_COORD, "Protocolo recibido: %d", protocolo_recibido);

	switch(protocolo_recibido){
	case 2:
		set_resultado_instancia(solicitud, OPERACION_EXITOSA);
		break;
	case OPERACION_EXITOSA:
		if(solicitud->instruccion == OPERACION_SET || solicitud->instruccion == CREAR_CLAVE)
			instancia->entradas_disponibles = recibir_entradas(instancia);

		if(solicitud->instruccion == STATUS)
			solicitud->valor = recibir_string(instancia->socket_instancia);

		set_resultado_instancia(solicitud, OPERACION_EXITOSA);
		break;
	case FS_NC:
		set_resultado_instancia(solicitud, FS_NC);
		break;
	case FS_EI:
		set_resultado_instancia(solicitud, FS_EI);
		break;
	case CLAVES_REEMPLAZADAS:
		actualizar_tablas_y_reintentar(solicitud, instancia);
		break;
	default:
		if(solicitud->instruccion != COMPACTACION){
			set_resultado_instancia(solicitud, ERROR_CLAVE_INACCESIBLE);

			agregar_clave_a_borrar(instancia, solicitud->clave);
		} else
			set_resultado_instancia(solicitud, ERROR_DE_COMUNICACION);

		desconectar_instancia(instancia);
	}
}

void actualizar_tablas_y_reintentar(t_solicitud* solicitud, t_instancia* instancia){
	t_list* claves = recibir_claves(instancia);

	borrar_claves(instancia, claves);

	list_destroy_and_destroy_elements(claves, free);

	evaluar_resultado_instr(solicitud, instancia);
}

void borrar_claves(t_instancia* instancia, t_list* claves){

	void remover_clave(char* clave){

		bool misma_clave(char* clave_instancia){
			return string_equals(clave_instancia, clave);
		}

		list_remove_by_condition(instancia->claves, (bool(*)(void*)) misma_clave);

		//todo modificar el espacio del array
	}

	list_iterate(claves, (void(*)(void*))remover_clave);
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

	case STATUS:{
		mensaje = serializar_status(solicitud);
		break;
	}

	default:
		return -1;
	}

	return enviar_mensaje(mensaje, socket);
}




