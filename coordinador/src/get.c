/*
 * get.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "get.h"

int get(t_solicitud* solicitud){
	if(validar_get_clave(solicitud) == -1)//FIXME capaz no hace falta chequear
		return -1;

	if(enviar_a_planif(solicitud) == -1)
		return -1;

	return 0;
}


t_solicitud* crear_get(int socket, int id){
	t_solicitud* solicitud = crear_solicitud(OPERACION_GET, id, socket);

	solicitud->clave = recibir_string(socket);

	return solicitud;
}

t_mensaje serializar_get_a_instancia(char* clave){
	int tam_clave = string_size(clave);

	t_mensaje mensaje = crear_mensaje(CREAR_CLAVE, sizeof(int) + tam_clave);

	serializar_string(mensaje.payload, clave);

	return mensaje;
}

t_mensaje serializar_clave_a_planif(t_solicitud* solicitud){
	// GET | id_esi + tam_clave + clave
	int tam_clave = string_size(solicitud->clave);

	t_mensaje mensaje = crear_mensaje(protocolo_planif(solicitud), sizeof(int) * 2 + tam_clave);

	char* aux = mensaje.payload;

	memcpy(aux, &solicitud->id_esi, sizeof(int));
	aux += sizeof(int);

	memcpy(aux, &tam_clave, sizeof(int));
	aux += sizeof(int);

	memcpy(aux, solicitud->clave, tam_clave);

	return mensaje;
}

int validar_get_clave(t_solicitud* solicitud){
	t_instancia* instancia = instancia_con_clave(solicitud);

	if(instancia == NULL){
		t_instancia* instancia_elegida = distribucion.algoritmo(solicitud->clave);

		if(instancia_elegida == NULL){
			log_error(LOG_COORD, "No se pudo aplicar el algoritmo de distribucion sobre la clave %s", solicitud->clave);

			return -1;
		}

		agregar_clave_a_crear(instancia_elegida, solicitud->clave);

		log_info(LOG_COORD,
			"Se aplico el algoritmo de distribucion sobre la clave %s, se creara en la instancia %d",
			solicitud->clave,
			instancia_elegida->id);
	} else {
		log_info(LOG_COORD, "La clave %s se encuentra en la instancia %d", solicitud->clave, instancia->id);

		actualizar_referencias_a_clave(instancia, solicitud);
	}

	return 0;
}

bool existe_clave_en_instancia_activa(t_solicitud* solicitud){
	t_instancia* instancia = instancia_con_clave(solicitud);

	return instancia != NULL && esta_activa(instancia);
}

void abortar_esi(t_solicitud* solicitud){
	if(enviar_paquete(solicitud->respuesta_a_esi, (int) solicitud->socket_esi, 0, NULL) <= 0){
		log_error(
			LOG_COORD,
			"No se pudo enviar el resultado de la instruccion %d al esi %d",
			solicitud->instruccion,
			solicitud->id_esi
		);
	}
}





