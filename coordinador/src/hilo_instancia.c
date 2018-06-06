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

void* crear_instancia(int id, int socket){
	t_instancia* instancia = malloc(sizeof(t_instancia));

	instancia->id = id;
	instancia->pedidos = queue_create();
	sem_init(&instancia->sem, 0, 0);
	instancia->socket = socket;
	instancia->esta_activa = true;
	instancia->claves = list_create();
	instancia->claves_a_borrar = list_create();

	return instancia;
}

int recibir_claves(t_instancia* instancia){
	int cant_claves;

	if(recv(instancia->socket, &cant_claves, sizeof(int), MSG_WAITALL) <= 0){
		log_error(LOG_COORD, "no se pudo recibir la cantidad de claves de la instancia %d", instancia->id);
		return -1;
	}

	int i;
	for(i = 0; i < cant_claves; i++){
		char* clave = recibir_string(instancia->socket);
		agregar_clave(instancia, clave);
	}

	return 0;
}

void* atender_instancia(void* instancia_void){
	t_instancia* instancia = (t_instancia*)instancia_void;
	t_solicitud* solicitud;

	if(enviar_config_instancia(instancia->socket) == -1){
		log_error(LOG_COORD, "no se pudo enviar la config a la instancia %d", instancia->id);
		return -1;
	}

	if(recibir_claves(instancia) == -1){
		log_error(LOG_COORD, "no se pudieron recibir las claves de la instancia %d", instancia->id);
		return -1;
	}

	while(true) {
		sem_wait(&instancia->sem);
		solicitud = (t_solicitud*)queue_pop(instancia->pedidos);

		if(enviar_pedido(solicitud, instancia->socket) == -1){
			log_error(
					LOG_COORD,
					"no se pudo enviar el pedido de la instruccion %d a la instancia %d",
					solicitud->instruccion,
					instancia->id
			);

			return -1;
		}

		evaluar_resultado_instr(solicitud, instancia->socket);//TODO checkear error
	}

	return 0;
}

void evaluar_resultado_instr(t_solicitud* solicitud, int socket_instancia){
	switch(recibir_protocolo(socket_instancia)){
	case OPERACION_EXITOSA:
		solicitud->resultado_instancia = OPERACION_EXITOSA;
		sem_post(&solicitud->solicitud_finalizada);
	break;
	default:;
	}
}

int enviar_config_instancia(int socket_instancia){
	int tamanio_payload = sizeof(CANTIDAD_ENTRADAS_TOTALES) + sizeof(TAMANIO_ENTRADA);
	void* payload = malloc(tamanio_payload);

	memcpy(payload, &CANTIDAD_ENTRADAS_TOTALES, sizeof(CANTIDAD_ENTRADAS_TOTALES));
	memcpy(payload + sizeof(CANTIDAD_ENTRADAS_TOTALES), &TAMANIO_ENTRADA, sizeof(TAMANIO_ENTRADA));

	return enviar_paquete(CONFIGURACION_ENTRADAS, socket_instancia, tamanio_payload, payload);
}

int enviar_pedido(t_solicitud* solicitud, int socket){

	switch(solicitud->instruccion){
	case OPERACION_GET:{
		t_mensaje mensaje = serializar_get_a_instancia(solicitud->clave);

		return enviar_mensaje(mensaje, socket);

		break;
	}
	case OPERACION_SET:{
		t_mensaje mensaje = serializar_set_a_instancia(solicitud->clave, solicitud->valor);

		return enviar_mensaje(mensaje, socket);

		break;
	}
	case OPERACION_STORE:{
		t_mensaje mensaje = serializar_store_a_instancia(solicitud->clave);

		return enviar_mensaje(mensaje, socket);

		break;
	}
	default:
		return -1;
	}

}

