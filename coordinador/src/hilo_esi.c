/*
 * hilo_esi.c
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#include "hilo_esi.h"

void crear_hilo_esi(int socket_cliente){

	crear_hilo(atender_esi, (void*) socket_cliente);

}

void* atender_esi(void* socket_esi){
	int id_esi = recibir_id(socket_esi);

	if(id_esi == -1){
		log_error(LOG_COORD, "ERROR: no se pudo recibir el id del esi en el socket %d", socket_esi);
		return -1;
	}

	while(true){
		t_solicitud* solicitud = recibir_solicitud_esi((int) socket_esi, id_esi);

		realizar_solicitud(solicitud); //TODO chequear si necesita error
	}

	return 0;
}

void realizar_solicitud(t_solicitud* solicitud){
	switch(solicitud->instruccion){
	case OPERACION_GET:
		realizar_get(solicitud);
		break;
	case OPERACION_SET:
		realizar_set(solicitud);
		break;
	case OPERACION_STORE:
		realizar_store(solicitud);
		break;
	}
}

int enviar_a_planif(int protocolo, void* payload, int tam_payload){
	pthread_mutex_lock(&SEM_SOCKET_PLANIF);

	int resultado_envio = enviar_paquete(protocolo, SOCKET_PLANIF, tam_payload, payload);

	pthread_mutex_unlock(&SEM_SOCKET_PLANIF);

	return resultado_envio;
}

t_list* instancias_con_clave(t_solicitud* solicitud){

	bool instancia_contiene_clave(t_instancia* instancia){
		bool contiene_clave(char* clave){
			return string_equals_ignore_case(clave, solicitud->clave);
		}

		return list_any_satisfy(instancia->claves, contiene_clave);
	}

	return list_filter(INSTANCIAS, instancia_contiene_clave);
}

bool instancia_activa(t_instancia* instancia){
	return instancia->esta_activa;
}

void crear_clave_con_instancia_caida(t_solicitud* solicitud, t_list* instancias){
	/**
	 * hay que usar el algoritmo de distribucion para elegir
	 * en que instancia se va a crear la clave nueva, y setear
	 * en todas las instancias un flag que indique que cuando
	 * una de ellas vuelva a conectarse, tiene que borrar una
	 * clave.
	 *
	 * Aca surge el problema de que hay que guardar en
	 * algun lado cual es la clave que se debe borrar.
	 *
	 * Una solucion posible es usar una estructura t_clave, que
	 * tenga un flag que indique si tiene que ser borrada o no.
	 * Otra solucion es una lista de claves que deben ser borradas
	 * en el t_instancia.
	 *
	 * Usar un t_clave puede ser mucho refactor, capaz rinde mas
	 * la lista de claves. Lo malo de la lista de claves es que, para
	 * borrar una clave, habria que tocar 2 listas al mismo tiempo
	 */
}

void crear_clave(t_solicitud* solicitud){
	distribuir(solicitud);//FIXME creo que es esto solo :O
}

void validar_existencia_clave(t_solicitud* solicitud){
	t_list* instancias = instancias_con_clave(solicitud);

	if(!list_is_empty(instancias))
		if(!list_any_satisfy(instancias, instancia_activa))
			crear_clave_con_instancia_caida(solicitud, instancias);
	else
		crear_clave(solicitud);
}

int realizar_get(t_solicitud* solicitud){
	//[GET_CLAVE | id, tam_clave, clave]

	validar_existencia_clave(solicitud);

	if(enviar_get_a_planif(solicitud) <= 0){
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

	if(enviar_resultado_get_esi(solicitud->socket_esi, resultado_get) <= 0){
		log_error(LOG_COORD, "no se pudo enviar el resultado del get de la clave %s al esi %d",
				solicitud->clave,
				solicitud->id_esi
		);
		return 1 * -1;
	}

}

int enviar_get_a_planif(t_solicitud* solicitud){
	int tam_clave = strlen(solicitud->clave) + 1;
	int tam_payload = sizeof(int) * 3 + tam_clave;

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

void distribuir(t_solicitud* solicitud){
	t_instancia* instancia = elegir_instancia(solicitud);//TODO mock

	agregar_pedido(instancia, solicitud);
	sem_post(&instancia->sem);
	//enviar_solicitud(solicitud, instancia);//TODO mock
}

t_instancia* elegir_instancia(t_solicitud* solicitud){//TODO mock
	t_instancia* instancia = NULL;

	return instancia;
}

void agregar_pedido(t_instancia* instancia, t_solicitud* solicitud){
	//faltarian semaforos
	list_add(instancia->pedidos, solicitud);
}

t_solicitud* recibir_solicitud_esi(int socket, int id){

	int protocolo = recibir_protocolo(socket);

	t_solicitud* solicitud;

	if(protocolo == -1){
		log_error(LOG_COORD, "El esi %d envio una solicitud invalida", socket);
		pthread_exit(NULL);
	}

	switch(protocolo){

	case OPERACION_GET:
		solicitud = crear_get(socket);
		break;
	case OPERACION_SET:
		solicitud = crear_set(socket);
		break;
	case OPERACION_STORE:
		solicitud = crear_store(socket);
		break;
	default:
		log_error(LOG_COORD, "El protocolo %d no existe", protocolo);
		pthread_exit(NULL);
		return NULL;
	}

	solicitud->id_esi = id;
	solicitud->socket_esi = socket;

	return solicitud;

}

t_solicitud* crear_get(int socket){
	t_solicitud* solicitud = malloc(sizeof(t_solicitud));

	solicitud->instruccion = OPERACION_GET;
	solicitud->clave = recibir_string(socket);

	return solicitud;
}

t_solicitud* crear_set(int socket){
	t_solicitud* solicitud = malloc(sizeof(t_solicitud));

	solicitud->instruccion = OPERACION_SET;
	solicitud->clave = recibir_string(socket);
	solicitud->valor = recibir_string(socket);

	return solicitud;
}

t_solicitud* crear_store(int socket){
	t_solicitud* solicitud = malloc(sizeof(t_solicitud));

	solicitud->instruccion = OPERACION_GET;
	solicitud->clave = recibir_string(socket);

	return solicitud;
}



int recibir_id(int socket_esi){
	int protocolo = recibir_protocolo(socket_esi);

	if(protocolo != ENVIO_ID)
		return -1;

	int id;

	int bytes_recibidos = recv(socket_esi, &id, sizeof(int), MSG_WAITALL);

	if(bytes_recibidos <= 0)
		return -1;

	return id;
}



























