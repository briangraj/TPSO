/*
 * get.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "get.h"

t_solicitud* crear_get(int socket){
	t_solicitud* solicitud = malloc(sizeof(t_solicitud));

	solicitud->instruccion = OPERACION_GET;
	solicitud->clave = recibir_string(socket);

	return solicitud;
}

t_mensaje serializar_get(char* clave){
	t_mensaje mensaje;

	mensaje.header = CREAR_CLAVE;

	int tam_clave = strlen(clave) + 1;

	mensaje.tam_payload = sizeof(int) + tam_clave;

	mensaje.payload = malloc(mensaje.tam_payload);

	serializar_string(mensaje.payload, clave);

	return mensaje;
}

int realizar_get(t_solicitud* solicitud){
	//[GET_CLAVE | id, tam_clave, clave]

	validar_existencia_clave(solicitud);

	if(enviar_get_a_planif(solicitud) < 0){
		log_error(LOG_COORD, "no se pudo enviar el get del esi %d de la clave %s al planificador",
				solicitud->id_esi,
				solicitud->clave
		);
		return 1 * -1;
	}

	int resultado_get = recibir_protocolo(SOCKET_PLANIF);

	if(resultado_get <= 0){
		log_error(LOG_COORD, "no se pudo recibir el resultado del get del esi %d de la clave %s",
				solicitud->id_esi,
				solicitud->clave
		);
		return 1 * -1;
	}

	if(enviar_paquete(resultado_get, solicitud->socket_esi, 0, NULL) <= 0){
		log_error(LOG_COORD, "no se pudo enviar el resultado del get de la clave %s al esi %d",
				solicitud->clave,
				solicitud->id_esi
		);
		return 1 * -1;
	}

}

int enviar_get_a_planif(t_solicitud* solicitud){
	int tam_clave = strlen(solicitud->clave) + 1;
	int tam_payload = sizeof(int) * 2 + tam_clave;

	void* payload = malloc(tam_payload);

	char* aux = payload;

	memcpy(aux, &solicitud->id_esi, sizeof(int));
	aux += sizeof(int);

	memcpy(aux, &tam_clave, sizeof(int));
	aux += sizeof(int);

	memcpy(aux, solicitud->clave, tam_clave);

	int resultado_envio = enviar_a_planif(GET_CLAVE, payload, tam_payload);

	if(resultado_envio <= 0)
		return -1;

	return 0;
}

void agregar_clave_a_borrar(t_solicitud* solicitud, t_list* instancias){
	bool no_contiene_clave_a_borrar(char* clave){
		return strcmp(clave, solicitud->clave);
	}

	bool no_contiene_clave_en_claves_a_borrar(t_instancia* instancia){
		return list_any_satisfy(instancia->claves_a_borrar, no_contiene_clave_a_borrar);
	}

	t_instancia* instancia = list_find(instancias, no_contiene_clave_en_claves_a_borrar);
}

void crear_clave_con_instancia_caida(t_solicitud* solicitud, t_list* instancias){
	distribuir(solicitud);

	agregar_clave_a_borrar(solicitud, instancias);
}

void crear_clave(t_solicitud* solicitud){
	distribuir(solicitud);
}

void validar_existencia_clave(t_solicitud* solicitud){
	t_list* instancias = instancias_con_clave(solicitud);

	if(!list_is_empty(instancias)){
		if(!list_any_satisfy(instancias, instancia_activa))
			crear_clave_con_instancia_caida(solicitud, instancias);
	}
	else
		crear_clave(solicitud);
}

t_list* instancias_con_clave(t_solicitud* solicitud){

	bool instancia_contiene_clave(t_instancia* instancia){
		bool contiene_clave(char* clave){
			return !strcmp(clave, solicitud->clave);
		}

		return list_any_satisfy(instancia->claves, contiene_clave);
	}

	return list_filter(INSTANCIAS, instancia_contiene_clave);
}

bool instancia_activa(t_instancia* instancia){
	return instancia->esta_activa;
}
