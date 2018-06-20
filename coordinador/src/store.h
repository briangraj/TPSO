/*
 * store.h
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#ifndef STORE_H_
#define STORE_H_

#include "coordinador.h"
#include "t_mensaje.h"
#include "conexiones/estructuras_coord.h"
#include "t_solicitud.h"

t_solicitud*	crear_store						(int socket, int id);
t_mensaje		serializar_store_a_instancia	(t_solicitud* solicitud);
int				store							(t_solicitud* solicitud);
t_mensaje 		serializar_store_a_planif		(t_solicitud* solicitud);

#endif /* STORE_H_ */
