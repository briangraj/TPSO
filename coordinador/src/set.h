/*
 * set.h
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#ifndef SET_H_
#define SET_H_

#include "coordinador.h"
#include "conexiones/estructuras_coord.h"
#include "t_mensaje.h"
#include "t_solicitud.h"
#include "t_instancia.h"

t_solicitud*	crear_set			   		(int socket, int id);
int				realizar_set		   		(t_solicitud* solicitud);
t_mensaje		serializar_set_a_instancia	(char* clave, char* valor);

#endif /* SET_H_ */
