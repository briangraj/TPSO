/*
 * set.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "set.h"

int set(t_solicitud* solicitud){
	t_instancia* instancia = instancia_con_clave(solicitud);

	if(checkear_clave_valida(instancia, solicitud) == -1)
		return -1;

	if(contiene_clave(instancia->claves_a_crear, solicitud))
		solicitud->instruccion = CREAR_CLAVE;

	if(enviar_a_planif(solicitud) == -1)
		return -1;

	if(solicitud->respuesta_a_esi == SET_EXITOSO){
		if(ejecutar(solicitud, instancia) == -1)
			return -1;

		actualizar_claves(instancia, solicitud);
	}

	return 0;
}

t_solicitud* crear_set(int socket, int id){
	t_solicitud* solicitud = crear_solicitud(OPERACION_SET, id, socket);

	solicitud->clave = recibir_string(socket);
	solicitud->valor = recibir_string(socket);

	return solicitud;
}

t_mensaje serializar_set_a_instancia(t_solicitud* solicitud){
	int tam_clave = string_size(solicitud->clave);
	int tam_valor = string_size(solicitud->valor);

	int tam_payload = sizeof(int) + tam_clave + sizeof(int) + tam_valor;

	t_mensaje mensaje = crear_mensaje(solicitud->instruccion, tam_payload);

	serializar_string(mensaje.payload, solicitud->clave);

	serializar_string(mensaje.payload + sizeof(int) + tam_clave, solicitud->valor);

	return mensaje;
}

t_mensaje serializar_set_a_planif(t_solicitud* solicitud){
	int tam_payload = sizeof(int) * 3 + string_size(solicitud->clave) + string_size(solicitud->valor);
	t_mensaje mensaje = crear_mensaje(SET_CLAVE, tam_payload);

	char* aux = mensaje.payload;

	memcpy(aux, &solicitud->id_esi, sizeof(int));
	aux += sizeof(int);

	serializar_string(aux, solicitud->clave);
	aux += string_size(solicitud->clave) + sizeof(int);

	serializar_string(aux, solicitud->valor);

	return mensaje;
}

void actualizar_claves(t_instancia* instancia, t_solicitud* solicitud){

	if(contiene_clave(instancia->claves_a_crear, solicitud)){
		bool es_la_clave(char* clave){
			return string_equals(clave, solicitud->clave);
		}

		list_remove_by_condition(instancia->claves_a_crear, (bool (*)(void*)) es_la_clave);

		list_add(instancia->claves, solicitud->clave);

	}

}

int validar_resultado_instancia(t_solicitud* solicitud, t_instancia* instancia){
	switch(solicitud->resultado_instancia){

	case ERROR_CLAVE_INACCESIBLE:
		setear_error_clave_inaccesible(solicitud);
		log_error_comunicacion_instancia(solicitud);

		return -1;
	case FS_NC:
		compactar_instancias();
		if(!instancia->esta_activa)
			return -1;
		ejecutar(solicitud, instancia);

		return 0;
	case FS_EI:
		log_error(LOG_COORD, "La instancia no tiene espacio para guardar el valor del set");
		solicitud->respuesta_a_esi = FS_EI;

		return -1;
	default:
		return 0;
	}
}

int resultado_enviar_a_planif(t_mensaje mensaje, t_solicitud* solicitud){
	int resultado_enviar_a_planif = enviar_mensaje(mensaje, SOCKET_PLANIF);

	destruir_mensaje(mensaje);

	if(resultado_enviar_a_planif < 0){

		log_error_envio_planif(solicitud);

		solicitud->respuesta_a_esi = ERROR_DE_COMUNICACION;

		desconectar_planif();

		return -1;
	}

	return 0;
}

int validar_resultado_planif(t_solicitud* solicitud){

	int resultado_planif = recibir_protocolo(SOCKET_PLANIF);

	if(resultado_planif <= 0){
		log_error_resultado_planif(solicitud);

		solicitud->respuesta_a_esi = ERROR_DE_COMUNICACION;

		desconectar_planif();

		return -1;
	}

	solicitud->respuesta_a_esi = resultado_planif;

	return 0;
}

