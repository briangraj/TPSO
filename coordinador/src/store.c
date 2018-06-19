/*
 * store.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "store.h"

int store(t_solicitud* solicitud){

	t_instancia* instancia = instancia_con_clave(solicitud);


	if(checkear_clave_valida(instancia, solicitud) == -1)
		return -1;


	agregar_solicitud(instancia, solicitud);


	sem_wait(&solicitud->solicitud_finalizada);


	if(validar_comunicacion_instancia(solicitud) == -1)
		return -1;


	t_mensaje store = serializar_store_a_planif(solicitud);


	if(resultado_enviar_a_planif(store, solicitud) == -1)
		return -1;


	log_trace(LOG_COORD, "Se envio el store %s al planificador", solicitud->clave);


	if(validar_resultado_planif(solicitud) == -1)
		return -1;


	log_info(LOG_COORD, "El resultado de ejecutar la instruccion %d fue %d",
			solicitud->instruccion,
			solicitud->respuesta_a_esi);


	return 0;

}

t_solicitud* crear_store(int socket, int id){
	t_solicitud* solicitud = crear_solicitud(OPERACION_STORE, id, socket);

	solicitud->clave = recibir_string(socket);

	return solicitud;
}

t_mensaje serializar_store_a_instancia(t_solicitud* solicitud){
	int tam_clave = strlen(solicitud->clave) + 1;

	t_mensaje mensaje = crear_mensaje(solicitud->instruccion, sizeof(int) + tam_clave);

	serializar_string(mensaje.payload, solicitud->clave);

	return mensaje;
}
