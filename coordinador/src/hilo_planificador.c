/*
 * hilo_planificador.c
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#include "hilo_planificador.h"

void crear_hilo_planificador(int socket_cliente){

	crear_hilo(atender_planificador, socket_cliente);
	//TODO desarrollar mas adelante
}


void* atender_planificador(void* socket_cliente){
	//TODO desarrollar mas adelante
}
