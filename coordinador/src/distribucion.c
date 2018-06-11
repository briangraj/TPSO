/*
 * distribucion.c
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#include "distribucion.h"


t_instancia* distribuir(t_solicitud* solicitud){//TODO mock
	t_instancia* instancia = (t_instancia*) list_get(INSTANCIAS, rand() % list_size(INSTANCIAS));

	return instancia;
}
