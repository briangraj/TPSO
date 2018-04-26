/*
 * hilo_instancia.h
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#ifndef HILO_INSTANCIA_H_
#define HILO_INSTANCIA_H_

#include "coordinador.h"

void crear_hilo_instancia(t_instancia* instancia);
void* atender_instancia(void* socket_instancia);
void enviar_configuracion_instancia(int socket_instancia);
int recibir_id(int socket);
void crear_instancia(int id, int socket);

#endif /* HILO_INSTANCIA_H_ */