/*
 * hilo_instancia.h
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#ifndef HILO_INSTANCIA_H_
#define HILO_INSTANCIA_H_

#include "coordinador.h"

void crear_hilo_instancia(int socket_instancia);

void* atender_instancia(void* socket_instancia);

void enviar_configuracion_instancia(int socket_instancia);

#endif /* HILO_INSTANCIA_H_ */
