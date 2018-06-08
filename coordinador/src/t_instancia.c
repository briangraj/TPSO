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
	instancia->pedidos = queue_create();
	sem_init(&instancia->sem, 0, 0);
	instancia->socket = socket;
	instancia->esta_activa = true;
	instancia->claves = list_create();
	instancia->claves_a_borrar = list_create();

	return instancia;
}

void agregar_clave(t_instancia* instancia, char* clave){
	list_add(instancia->claves, clave);
}

bool es_instancia_activa(t_instancia* instancia){
	return instancia->esta_activa;
}

void agregar_pedido(t_instancia* instancia, t_solicitud* solicitud){
	queue_push(instancia->pedidos, solicitud);

	sem_post(&instancia->sem);
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
