/*
 * hilo_instancia.h
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#ifndef HILO_INSTANCIA_H_
#define HILO_INSTANCIA_H_

#include "coordinador.h"
#include "t_instancia.h"

void  crear_hilo_instancia			 (t_instancia* instancia);
void* crear_instancia				 (int id, int socket);
int   recibir_id					 (int socket);
void* atender_instancia				 (void* socket_instancia);
int   enviar_config_instancia        (int socket_instancia);
int   enviar_pedido					 (t_solicitud* pedido, int socket);

#endif /* HILO_INSTANCIA_H_ */
