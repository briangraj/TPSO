/*
 * t_instancia.c
 *
 *  Created on: 4 jun. 2018
 *      Author: utnso
 */

#include "t_instancia.h"

void agregar_clave(t_instancia* instancia, char* clave){
	/**
	 * FIXME la coleccion de claves conviene mas que sea un t_list* o un diccionario?
	 */
	list_add(instancia->claves, clave);
}

bool es_instancia_activa(t_instancia* instancia){
	return instancia->esta_activa;
}

void agregar_pedido(t_instancia* instancia, t_solicitud* solicitud){
	queue_push(instancia->pedidos, solicitud);

	sem_post(&instancia->sem);
}
