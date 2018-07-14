/*
 * compactacion.c
 *
 *  Created on: 9 jul. 2018
 *      Author: utnso
 */

#include "compactacion.h"

t_list* instancias_activas(){
	return list_filter(INSTANCIAS, (bool(*)(void*)) esta_activa);
}

void compactar_instancias(){
	list_iterate(instancias_activas(), (void(*)(void*)) compactar);
}

void compactar(t_instancia* instancia){
	t_solicitud* solicitud = crear_solicitud(COMPACTACION, -1, -1);

	agregar_solicitud(instancia, solicitud);

	sem_wait(&solicitud->solicitud_finalizada);

	sem_close(&solicitud->solicitud_finalizada);
	sem_destroy(&solicitud->solicitud_finalizada);
	free(solicitud);
}

t_mensaje serializar_compactacion(t_solicitud* solicitud){
	return crear_mensaje(COMPACTACION, 0);
}
