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
#include "get.h"

t_solicitud*	crear_set			   			(int socket, int id);
int				set		   			(t_solicitud* solicitud);
t_mensaje		serializar_set_a_instancia		(char* clave, char* valor);
t_mensaje		serializar_clave_a_planif			(t_solicitud* solicitud);
void 			actualizar_claves				(t_instancia* instancia, t_solicitud* solicitud);
int 			validar_comunicacion_instancia	(t_solicitud* solicitud);
int 			checkear_clave_valida		(t_instancia* instancia, t_solicitud* solicitud);
int 			resultado_enviar_a_planif		(t_mensaje* set, t_solicitud* solicitud);
int 			validar_resultado_planif		(t_solicitud* solicitud);

#endif /* SET_H_ */
