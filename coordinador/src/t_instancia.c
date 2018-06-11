/*
 * t_instancia.c
 *
 *  Created on: 4 jun. 2018
 *      Author: utnso
 */

#include "t_instancia.h"

void* crear_instancia(int id, int socket){
	t_instancia* instancia = malloc(sizeof(t_instancia));

	instancia->id = id;
	instancia->solicitudes = queue_create();
	sem_init(&instancia->solicitud_lista, 0, 0);
	instancia->socket = socket;
	instancia->esta_activa = true;
	instancia->claves = list_create();
	instancia->claves_a_crear = list_create();

	return instancia;
}

t_instancia* instancia_con_clave(t_solicitud* solicitud){

	bool instancia_contiene_clave(t_instancia* instancia){
		bool contiene_clave(char* clave){
			return !strcmp(clave, solicitud->clave);
		}

		return list_any_satisfy(instancia->claves, contiene_clave);
	}

	return (t_instancia*) list_find(INSTANCIAS, instancia_contiene_clave);
}

void borrar_clave(t_solicitud* solicitud, t_instancia* instancia){
	bool contiene_clave(char* clave){
		return !strcmp(clave, solicitud->clave);
	}

	char* clave = (char*) list_remove_by_condition(instancia->claves, contiene_clave);

//	list_add(instancia->claves_a_borrar, (void*) clave);
}

void agregar_clave(t_instancia* instancia, char* clave){
	list_add(instancia->claves, clave);
}

void agregar_clave_a_crear(t_instancia* instancia, char* clave){
	list_add(instancia->claves_a_crear, clave);
}

bool esta_activa(t_instancia* instancia){
	return instancia->esta_activa;
}

t_solicitud* sacar_solicitud(t_instancia* instancia) {
	return (t_solicitud*) queue_pop(instancia->solicitudes);
}

void agregar_solicitud(t_instancia* instancia, t_solicitud* solicitud){
	queue_push(instancia->solicitudes, solicitud);

	sem_post(&instancia->solicitud_lista);
}

int recibir_claves(t_instancia* instancia){
	int cant_claves;

	if(recv(instancia->socket, &cant_claves, sizeof(int), MSG_WAITALL) <= 0){
		log_error(LOG_COORD, "No se pudo recibir la cantidad de claves de la instancia %d", instancia->id);
		return -1;
	}

	log_info(LOG_COORD, "Se recibieron las claves de la instancia");

	int i;
	for(i = 0; i < cant_claves; i++){
		char* clave = recibir_string(instancia->socket);
		agregar_clave(instancia, clave);
	}

	log_info(LOG_COORD, "Se agregaron las claves a la instancia %d", instancia->id);
	return 0;
}

void destruir_instancias(){
	list_iterate(INSTANCIAS, (void (*)(void*)) destruir_instancia);
}

void destruir_instancia(t_instancia* instancia){
	sem_close(&instancia->solicitud_lista);
	sem_destroy(&instancia->solicitud_lista);

	list_destroy_and_destroy_elements(instancia->claves, free);
	list_destroy_and_destroy_elements(instancia->claves_a_crear, free);

	queue_destroy_and_destroy_elements(instancia->solicitudes, destruir_solicitud);

	pthread_cancel(instancia->id_hilo);

	close(instancia->socket);
}
