/*
 * distribucion.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "distribucion.h"

void distribuir(t_solicitud* solicitud){
	t_instancia* instancia = elegir_instancia(solicitud);//TODO mock
	log_trace(LOG_COORD, "Se eligio la instancia de id %d", instancia->id);

	agregar_pedido(instancia, solicitud);
}

t_instancia* elegir_instancia(t_solicitud* solicitud){//TODO mock
	t_instancia* instancia = (t_instancia*) list_get(INSTANCIAS, rand() % list_size(INSTANCIAS));

	return instancia;
}
