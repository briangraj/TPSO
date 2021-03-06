/*
 * store.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "store.h"

int store(t_solicitud* solicitud){
	t_instancia* instancia = instancia_con_clave(solicitud);

	if(validar_op_con_efecto_sobre_clave(&instancia, solicitud) == -1)
		return -1;

	if(es_clave_a_crear(instancia, solicitud)){
		set_respuesta_a_esi(solicitud, STORE_INVALIDO);

		return -1;
	}

	if(enviar_a_planif(solicitud) == -1)
		return -1;

	if(solicitud->respuesta_a_esi == STORE_EXITOSO){
		if(ejecutar(solicitud, &instancia) == -1)
			return -1;
	}

	return 0;

}

t_solicitud* crear_store(int socket, int id){
	t_solicitud* solicitud = crear_solicitud(OPERACION_STORE, id, socket);

	solicitud->clave = recibir_string(socket);

	return solicitud;
}

t_mensaje serializar_store_a_instancia(t_solicitud* solicitud){
	int tam_clave = string_size(solicitud->clave);

	t_mensaje mensaje = crear_mensaje(solicitud->instruccion, sizeof(int) + tam_clave);

	serializar_string(mensaje.payload, solicitud->clave);

	return mensaje;
}
