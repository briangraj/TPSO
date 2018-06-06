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

t_solicitud* crear_store		(int socket);
t_mensaje 	 serializar_store	(char* clave);
int 		 realizar_store		(t_solicitud* solicitud);

#endif /* STORE_H_ */
