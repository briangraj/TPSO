/*
 * store.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "store.h"

int store(t_solicitud* solicitud){
	t_instancia* instancia = instancia_con_clave(solicitud);

	if(contiene_clave(instancia->claves_a_crear, solicitud)){
		set_respuesta_a_esi(solicitud, NO_SE_HIZO_UN_GET_ANTES);

		return -1;
	}

	if(checkear_clave_valida(instancia, solicitud) == -1)
		return -1;

	if(enviar_a_planif(solicitud) == -1)
		return -1;

	if(solicitud->respuesta_a_esi == STORE_EXITOSO){
		if(ejecutar(solicitud, instancia) == -1)
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
