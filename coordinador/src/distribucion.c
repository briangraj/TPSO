/*
 * distribucion.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "distribucion.h"


t_instancia* equitative_load(t_solicitud* solicitud){
	t_instancia* instancia = list_get(INSTANCIAS, distribucion.proxima_instancia);

	distribucion.proxima_instancia = (distribucion.proxima_instancia + 1) % list_size(INSTANCIAS);

	if(!esta_activa(instancia))
		return equitative_load(solicitud);

	return instancia;
}

t_instancia* least_space_used(t_solicitud* solicitud){
	t_instancia* instancia;
	return instancia;
}

t_instancia* key_explicit(t_solicitud* solicitud){
	t_instancia* instancia;
	return instancia;
}
