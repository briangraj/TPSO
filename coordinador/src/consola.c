/*
 * consola.c
 *
 *  Created on: 10 jul. 2018
 *      Author: utnso
 */
#include "consola.h"

void crear_hilo_consola(){
	crear_hilo(atender_consola, NULL);
}

void* atender_consola(void* _){
	// STATUS tamanio clave / clave
	while(1){
		int protocolo = recibir_protocolo(SOCKET_CONSOLA);

		if(protocolo < 0){
			PLANIF_CONECTADO = false;

			verificar_estado_valido();

			break;
		}

		if(protocolo != STATUS)
			break;

		t_status status = recibir_status();

		t_info_status info_status = armar_status(status);

		if(enviar_status(info_status) <= 0){
			log_error(LOG_COORD, "Se perdio la conexion con la consola");

			PLANIF_CONECTADO = false;

			verificar_estado_valido();

			break;
		}
	}

	pthread_exit(NULL);
}

int enviar_status(t_info_status info_status){
	int tam_payload;

	void* payload = serializar_info_status(&info_status, &tam_payload);

	return enviar_paquete(ENVIO_INFO_STATUS, SOCKET_CONSOLA, tam_payload, payload);
}

t_status recibir_status(){
	t_status status;

	if(recv(SOCKET_CONSOLA, &status.tamanio_clave, sizeof(int), MSG_WAITALL) <= 0)
		log_error(LOG_COORD, "No se pudo recibir el tamanio de la clave del status");

	status.clave = malloc(status.tamanio_clave);

	if(recv(SOCKET_CONSOLA, status.clave, status.tamanio_clave, MSG_WAITALL) <= 0)
		log_error(LOG_COORD, "No se pudo recibir la clave del status");

	return status;
}

t_solicitud* crear_status(t_status status){
	t_solicitud* solicitud = crear_solicitud(STATUS, -1, -1);

	solicitud->clave = status.clave;

	return solicitud;
}

t_info_status armar_status(t_status status){
	t_solicitud* solicitud = crear_status(status);

	t_instancia* instancia = instancia_con_clave(solicitud);

	t_info_status info_status;

	if(instancia == NULL){
		log_error(LOG_COORD, "La clave %s no se encontro en ninguna instancia", solicitud->clave);

		info_status = info_status_clave_inexistente(solicitud->clave);

		liberar_solicitud(solicitud);

		return info_status;
	}

	if(!esta_activa(instancia)){
		log_error(LOG_COORD, "La clave %s esta en una instancia inactiva", solicitud->clave);

		info_status = info_status_clave_inaccesible(instancia->id);

		liberar_solicitud(solicitud);

		return info_status;
	}

	if(es_clave_a_crear(instancia, solicitud)){
		info_status = info_status_clave_a_crear(instancia);
	} else
		info_status = enviar_status_a_instancia(instancia, solicitud);

	liberar_solicitud(solicitud);

	return info_status;
}

t_info_status enviar_status_a_instancia(t_instancia* instancia, t_solicitud* solicitud){
	agregar_solicitud(instancia, solicitud);

	sem_wait(&solicitud->solicitud_finalizada);

	if(solicitud->resultado_instancia == ERROR_CLAVE_INACCESIBLE){
		log_error(LOG_COORD, "La clave %s esta en una instancia inactiva", solicitud->clave);

		return info_status_clave_inaccesible(instancia->id);
	}

	return info_status_clave_existente(solicitud, instancia);
}

t_info_status info_status_clave_existente(t_solicitud* solicitud, t_instancia* instancia){
	log_warning(LOG_COORD, "valor: %s", solicitud->valor);
	return (t_info_status) {
			.tamanio_mensaje = string_length(solicitud->valor),
			.mensaje = strdup(solicitud->valor),
			.id_instancia_actual = instancia->id,
			.id_instancia_posible = instancia->id
	};
}

t_info_status info_status_clave_inaccesible(int id){
	return (t_info_status) {
			.tamanio_mensaje = string_size("CLAVE INACCESIBLE"),
			.mensaje = "CLAVE INACCESIBLE",
			.id_instancia_actual = id,
			.id_instancia_posible = -1
	};
}

t_info_status info_status_clave_a_crear(t_instancia* instancia){
	return (t_info_status) {
			.tamanio_mensaje = string_size("CLAVE SIN VALOR"),
			.mensaje = "CLAVE SIN VALOR",
			.id_instancia_actual = -1,
			.id_instancia_posible = instancia->id
	};
}

t_info_status info_status_clave_inexistente(char* clave){
	int proxima_instancia = distribucion.proxima_instancia;
	t_instancia* instancia = distribucion.algoritmo(clave);
	int id;

	if(instancia == NULL){
		id = -1;
	} else {
		id = instancia->id;
	}

	t_info_status info_status = {
			.tamanio_mensaje = string_size("CLAVE SIN VALOR"),
			.mensaje = "CLAVE SIN VALOR",
			.id_instancia_actual = -1,
			.id_instancia_posible = id
	};

	distribucion.proxima_instancia = proxima_instancia;

	return info_status;
}

t_mensaje serializar_status(t_solicitud* solicitud){

	t_mensaje mensaje = crear_mensaje(STATUS, sizeof(int) + string_size(solicitud->clave));

	serializar_string(mensaje.payload, solicitud->clave);

	return mensaje;

}
