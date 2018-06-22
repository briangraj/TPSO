/*
 * hilo_esi.h
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#ifndef HILO_ESI_H_
#define HILO_ESI_H_

#include "coordinador.h"
#include "t_mensaje.h"

void 		 crear_hilo_esi		   		(int socket_cliente);
void* 		 atender_esi		  		(void* socket_esi);
t_solicitud* recibir_solicitud_esi 		(int socket, int id);
int 		 atender_solicitud	  		(t_solicitud* solicitud);
int 		 recibir_id		   	   		(int socket);
void		 verificar_estado_valido	();
int			 ejecutar					(t_solicitud* solicitud, t_instancia* instancia);
int 		 enviar_a_planif			(t_solicitud* solicitud);
t_mensaje 	 serializar_a_planif		(t_solicitud* solicitud);

#endif /* HILO_ESI_H_ */
