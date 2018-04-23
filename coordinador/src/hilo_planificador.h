/*
 * hilo_planifador.h
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#ifndef HILO_PLANIFICADOR_H_
#define HILO_PLANIFICADOR_H_

#include "conexiones/threads.h"

void crear_hilo_planificador(int socket_cliente);

void* atender_planificador(void* socket_cliente);


#endif /* HILO_PLANIFICADOR_H_ */
