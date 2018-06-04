/*
 * get.h
 *
 *  Created on: 3 jun. 2018
 *      Author: utnso
 */

#ifndef GET_H_
#define GET_H_

#include "coordinador.h"
#include "conexiones/estructuras_coord.h"

t_solicitud* crear_get			   			(int socket);
void 		 realizar_get		   			(t_solicitud* solicitud);
int 		 enviar_get_a_planif   			(t_solicitud* solicitud);
void 		 agregar_clave_a_borrar			(t_solicitud* solicitud, t_list* instancias);
void 		 crear_clave_con_instancia_caida(t_solicitud* solicitud, t_list* instancias);
void 		 crear_clave					(t_solicitud* solicitud);
void 		 validar_existencia_clave		(t_solicitud* solicitud);
t_list* 	 instancias_con_clave			(t_solicitud* solicitud);
bool 		 instancia_activa				(t_instancia* instancia);
t_mensaje 	 serializar_get					(char* clave);

#endif /* GET_H_ */
