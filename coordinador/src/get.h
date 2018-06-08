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
#include "t_mensaje.h"
#include "t_instancia.h"
#include "t_solicitud.h"

t_solicitud* crear_get			   			(int socket, int id);
int 		 realizar_get		   			(t_solicitud* solicitud);
void 		 agregar_clave_a_borrar			(t_solicitud* solicitud, t_list* instancias);
void 		 crear_clave_con_instancia_caida(t_solicitud* solicitud, t_list* instancias);
void 		 crear_clave					(t_solicitud* solicitud);
void 		 validar_existencia_clave		(t_solicitud* solicitud);
t_list* 	 instancias_con_clave			(t_solicitud* solicitud);
t_mensaje 	 serializar_get_a_instancia		(char* clave);
t_mensaje 	 serializar_get_a_planif		(t_solicitud* solicitud);
int 		 evaluar_resultados				(int resultado_instancia, int resultado_planif);

#endif /* GET_H_ */
